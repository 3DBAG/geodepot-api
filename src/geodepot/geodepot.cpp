#include <geodepot/geodepot.h>
#include <archive.h>
#include <archive_entry.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#ifndef _WIN32
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif
namespace fs=std::filesystem; using json=nlohmann::json;
namespace {
[[noreturn]] void die(geodepot::ErrorCode c,const std::string& m){throw geodepot::Error(c,m);}
struct Hash { SHA256_CTX c{}; Hash(){SHA256_Init(&c);} void add(const void*p,size_t n){SHA256_Update(&c,p,n);} std::string done(){ unsigned char d[SHA256_DIGEST_LENGTH]; SHA256_Final(d,&c); static constexpr char x[]="0123456789abcdef"; std::string s(64,'0'); for(int i=0;i<32;++i){s[i*2]=x[d[i]>>4];s[i*2+1]=x[d[i]&15];} return s;} };
struct Digest {std::string hash; uint64_t size;};
Digest file_hash(const fs::path&p){std::ifstream f(p,std::ios::binary);if(!f)die(geodepot::ErrorCode::cache,"cannot read "+p.string());Hash h;std::array<char,65536>b{};uint64_t n=0;while(f.read(b.data(),b.size())||f.gcount()){auto k=(size_t)f.gcount();h.add(b.data(),k);n+=k;}return {h.done(),n};}
void u64(Hash&h,uint64_t v){unsigned char b[8];for(int i=7;i>=0;--i){b[i]=v&255;v>>=8;}h.add(b,8);}
Digest content_hash(const fs::path&p){if(fs::is_regular_file(p))return file_hash(p);if(!fs::is_directory(p))die(geodepot::ErrorCode::integrity,"data is not file or directory");std::vector<fs::path>v;for(auto&e:fs::recursive_directory_iterator(p)){if(e.is_directory())continue;if(e.is_symlink()||!e.is_regular_file())die(geodepot::ErrorCode::integrity,"directory contains link or special file");v.push_back(e.path());}std::sort(v.begin(),v.end(),[&](auto&a,auto&b){return a.lexically_relative(p).generic_string()<b.lexically_relative(p).generic_string();});Hash h;uint64_t total=0;for(auto&f:v){auto r=f.lexically_relative(p).generic_string();auto d=file_hash(f);u64(h,r.size());h.add(r.data(),r.size());u64(h,d.size);std::ifstream in(f,std::ios::binary);std::array<char,65536>b{};while(in.read(b.data(),b.size())||in.gcount())h.add(b.data(),in.gcount());total+=d.size;}return {h.done(),total};}
bool sha(const std::string&s){return s.size()==64&&std::all_of(s.begin(),s.end(),[](unsigned char c){return c>='0'&&c<='9'||c>='a'&&c<='f';});}
json load(const fs::path&p,geodepot::ErrorCode c){std::ifstream f(p);if(!f)die(c,"cannot read "+p.string());try{return json::parse(f);}catch(const json::exception&e){die(c,"invalid JSON: "+std::string(e.what()));}}
std::string req(const json&j,const char*k,geodepot::ErrorCode c){if(!j.contains(k)||!j[k].is_string())die(c,std::string("missing ")+k);return j[k].get<std::string>();}
uint64_t num(const json&j,const char*k){if(!j.contains(k)||!j[k].is_number_unsigned())die(geodepot::ErrorCode::metadata,std::string("missing ")+k);return j[k].get<uint64_t>();}
class Lock {
 public:
  explicit Lock(const fs::path& p) {
    fs::create_directories(p.parent_path());
#ifndef _WIN32
    fd = open(p.c_str(), O_CREAT | O_RDWR, 0600);
    if (fd < 0 || flock(fd, LOCK_EX)) die(geodepot::ErrorCode::cache, "cannot lock cache");
#endif
  }
  ~Lock() {
#ifndef _WIN32
    if (fd >= 0) { flock(fd, LOCK_UN); close(fd); }
#endif
  }
 private:
#ifndef _WIN32
  int fd = -1;
#endif
};
size_t write(char*b,size_t s,size_t n,void*u){return fwrite(b,s,n,(FILE*)u);}
void fetch(const std::string&url,const fs::path&dest,bool offline,const std::string*expected=nullptr,uint64_t size=0){if(offline){if(!fs::exists(dest))die(geodepot::ErrorCode::network,"offline cache miss");}else{bool ok=false;for(int i=0;i<3&&!ok;++i){auto tmp=dest.string()+".part";FILE*out=fopen(tmp.c_str(),"wb");if(!out)die(geodepot::ErrorCode::cache,"cannot create cache file");CURL*c=curl_easy_init();curl_easy_setopt(c,CURLOPT_URL,url.c_str());curl_easy_setopt(c,CURLOPT_WRITEFUNCTION,write);curl_easy_setopt(c,CURLOPT_WRITEDATA,out);curl_easy_setopt(c,CURLOPT_FOLLOWLOCATION,1L);curl_easy_setopt(c,CURLOPT_MAXREDIRS,5L);curl_easy_setopt(c,CURLOPT_CONNECTTIMEOUT,10L);curl_easy_setopt(c,CURLOPT_LOW_SPEED_LIMIT,1024L);curl_easy_setopt(c,CURLOPT_LOW_SPEED_TIME,30L);curl_easy_setopt(c,CURLOPT_PROTOCOLS_STR,"http,https");curl_easy_setopt(c,CURLOPT_REDIR_PROTOCOLS_STR,"http,https");auto r=curl_easy_perform(c);long status=0;curl_easy_getinfo(c,CURLINFO_RESPONSE_CODE,&status);curl_easy_cleanup(c);fclose(out);if(r==CURLE_OK&&status>=200&&status<300){fs::rename(tmp,dest);ok=true;}else{fs::remove(tmp);if(status==404)die(geodepot::ErrorCode::missing,"HTTP 404");if(!(r!=CURLE_OK||status==408||status==429||status>=500))die(geodepot::ErrorCode::network,"HTTP "+std::to_string(status));}}if(!ok)die(geodepot::ErrorCode::network,"download failed");}auto d=file_hash(dest);if((expected&&d.hash!=*expected)||(size&&d.size!=size)){fs::remove(dest);die(geodepot::ErrorCode::integrity,"download integrity failure");}}
struct Art{std::string c,d,h,ah;uint64_t n,an;};
void extract(const fs::path&tar,const fs::path&tmp,const Art&a){archive*ar=archive_read_new();archive_read_support_format_tar(ar);if(archive_read_open_filename(ar,tar.c_str(),10240)!=ARCHIVE_OK)die(geodepot::ErrorCode::integrity,"invalid archive");archive_entry*e;uint64_t expanded=0;while(archive_read_next_header(ar,&e)==ARCHIVE_OK){std::string name=archive_entry_pathname(e);fs::path rel(name);if(name.empty()||rel.is_absolute()||name.find('\\')!=std::string::npos||std::any_of(rel.begin(),rel.end(),[](auto&p){return p=="..";})||rel.begin()->string()!=a.d||archive_entry_symlink(e)||archive_entry_hardlink(e)||(archive_entry_filetype(e)!=AE_IFREG&&archive_entry_filetype(e)!=AE_IFDIR))die(geodepot::ErrorCode::integrity,"unsafe archive entry");if(archive_entry_filetype(e)==AE_IFDIR){fs::create_directories(tmp/rel);continue;}if((uint64_t)archive_entry_size(e)>a.n-expanded)die(geodepot::ErrorCode::integrity,"archive expansion exceeds data size");fs::create_directories((tmp/rel).parent_path());std::ofstream o(tmp/rel,std::ios::binary);std::array<char,65536>b{};la_ssize_t k;while((k=archive_read_data(ar,b.data(),b.size()))>0){o.write(b.data(),k);expanded+=k;}if(k<0||!o)die(geodepot::ErrorCode::integrity,"archive extraction failed");}archive_read_free(ar);}
}
namespace geodepot { fs::path prepare(const PrepareOptions&o){if(!fs::is_regular_file(o.lock_file)||o.cache_root.empty()||o.suite.empty())die(ErrorCode::cli,"invalid prepare options");auto l=load(o.lock_file,ErrorCode::cli);auto remote=req(l,"remote",ErrorCode::cli);while(remote.ends_with('/'))remote.pop_back();if(!remote.starts_with("http://")&&!remote.starts_with("https://"))die(ErrorCode::cli,"remote must be HTTP(S)");auto ih=req(l,"index_sha256",ErrorCode::cli);if(!sha(ih))die(ErrorCode::cli,"invalid index digest");Hash kh;kh.add(remote.data(),remote.size());auto base=fs::absolute(o.cache_root)/kh.done()/ih;fs::create_directories(base);Lock master(base/"prepare.lock");auto relp=base/"release.json",idxp=base/"index.geojson";fetch(remote+"/release.json",relp,o.offline);fetch(remote+"/.geodepot/index.geojson",idxp,o.offline,&ih);auto rel=load(relp,ErrorCode::metadata);if(req(rel,"version",ErrorCode::metadata)!=req(l,"version",ErrorCode::cli)||req(rel,"index_sha256",ErrorCode::metadata)!=ih||req(rel,"status",ErrorCode::metadata)!="complete")die(ErrorCode::metadata,"release does not match lock");if(!rel.contains("suites")||!rel["suites"].contains(o.suite))die(ErrorCode::missing,"suite not found");auto list=rel["suites"][o.suite];if(list.is_object()&&list.contains("casespecs"))list=list["casespecs"];if(!list.is_array())die(ErrorCode::metadata,"invalid suite");std::map<std::string,Art>arts;auto idx=load(idxp,ErrorCode::metadata);if(!idx.contains("features")||!idx["features"].is_array())die(ErrorCode::metadata,"invalid index");for(auto&f:idx["features"]){if(!f.contains("properties")||!f["properties"].is_object())die(ErrorCode::metadata,"invalid index feature");auto&p=f["properties"];auto c=req(p,"case_name",ErrorCode::metadata);auto d=req(p,"data_name",ErrorCode::metadata);Art a{c,d,req(p,"data_sha256",ErrorCode::metadata),req(p,"archive_sha256",ErrorCode::metadata),num(p,"data_size"),num(p,"archive_size")};if(!sha(a.h)||!sha(a.ah))die(ErrorCode::metadata,"invalid index digest");arts[c+"/"+d]=a;}for(auto&m:list){std::string key=m.is_string()?m.get<std::string>():req(m,"casespec",ErrorCode::metadata);if(!arts.contains(key))die(ErrorCode::metadata,"suite entry absent from index");auto&a=arts.at(key);auto final=base/"root"/a.c/a.d;Lock al(base/"locks"/(a.c+"--"+a.d+".lock"));if(fs::exists(final)){auto d=content_hash(final);if(d.hash==a.h&&d.size==a.n)continue;fs::remove_all(final);}auto tar=base/"archives"/a.c/(a.d+".tar");fs::create_directories(tar.parent_path());fetch(remote+"/.geodepot/cases/"+a.c+"/"+a.d+".tar",tar,o.offline,&a.ah,a.an);auto tmp=base/"tmp";fs::remove_all(tmp);fs::create_directories(tmp);extract(tar,tmp,a);auto d=content_hash(tmp/a.d);if(d.hash!=a.h||d.size!=a.n)die(ErrorCode::integrity,"content integrity failure");fs::create_directories(final.parent_path());fs::rename(tmp/a.d,final);fs::remove_all(tmp);}return fs::absolute(base/"root");} }
