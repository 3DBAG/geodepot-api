#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("../../include/geodepot/geodepot.h");

        type Repository;
        
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {

    }
}
