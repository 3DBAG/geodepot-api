import geodepot as m

def test_init():
    repo = m.Repository("/home/balazs/Development/geodepot-api/tests/data/mock_project/"
        ".geodepot")

def test_get():
    repo = m.Repository("/home/balazs/Development/geodepot-api/tests/data/mock_project/"
                        ".geodepot")
    p = repo.get("wippolder/wippolder.zip")