import geodepot as m

def test_init():
    assert m.init() == 0
def test_get():
    assert m.get() == 42