We cannot upload cannot upload a `linux_*.whl` wheel to PyPI (see [link](https://stackoverflow.com/a/59586096)).
I need to convert this platform-specific wheel into a manylinux wheel via the manylinux project and the auditwheel tool.
A very good explanation of why cannot we upload the linux platform specific wheels to PyPi is in [the PEP 513](https://peps.python.org/pep-0513/#rationale).

```shell
cibuildwheel >>cibuildwheel.log 2>&1
```

Creates sdist and wheels to `dist/`

```shell
uv build
```

Upload to TestPyPi.
TestPyPi credentials are set up in `$HOME/.pypirc`.

```shell
twine upload --repository testpypi dist/*
```

