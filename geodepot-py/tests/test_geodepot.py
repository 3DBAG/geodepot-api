import geodepot as m

def test_init():
    repo = m.Repository("/home/balazs/Development/geodepot-api/tests/data/mock_project")

def test_get():
    repo = m.Repository("/home/balazs/Development/geodepot-api/tests/data/mock_project")
    p = repo.get("wippolder/wippolder.zip")