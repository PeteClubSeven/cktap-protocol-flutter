SET scriptDir=%~dp0
SET rootDir="%scriptDir%\.."

IF EXIST "%rootDir%\.git" IF EXIST "%rootDir%\.git\HEAD" (
    PUSHD "%rootDir%"
    git submodule update --init --recursive
    POPD
)
