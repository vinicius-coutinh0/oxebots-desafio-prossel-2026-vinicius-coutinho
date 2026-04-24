@echo off
setlocal

echo [1/3] Criando pasta de build...
if not exist build (
    mkdir build
)

echo [2/3] Configurando o projeto com CMake...
cmake -B build -S . -DBUILD_TESTING=OFF
if %errorlevel% neq 0 (
    echo [ERRO] Falha na configuracao do CMake. Verifique se o CMake e o compilador C++ estao instalados.
    pause
    exit /b %errorlevel%
)

echo [3/3] Iniciando a compilacao (Release)...
cmake --build build --config Release
if %errorlevel% neq 0 (
    echo [ERRO] Falha durante a compilacao do projeto.
    pause
    exit /b %errorlevel%
)

echo ------------------------------------------------
echo Compilacao concluida com sucesso!
echo ------------------------------------------------
echo Para rodar o simulador:
echo .\build\Release\SimuladorFutebol.exe
echo ------------------------------------------------
pause
