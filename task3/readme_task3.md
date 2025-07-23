Решение написано на С++ и тестировалось на Ubuntu 25.04, я ожидаю, что проверяющий имеет такой же стенд/докер-образ. Я прикрепляю инструкции для компиляции на такой же сборке, и инструкцию для альтернативных платформ если сборка отличается от моей.

Linux (Ubuntu 25.04): установить библиотеки, обновить систему: sudo apt-get update &&
apt-get install -y
build-essential
cmake
git
curl
libcurl4-openssl-dev
libtinyxml2-dev
libiconv-hook-dev
g++
Выполнить в папке с main.cpp g++ main.cpp -o currency_rates -lcurl -ltinyxml2 -std=c++17 ./currency_rates

macOS, WSL, Windows Docker Desktop, Windows:
(перед этим установить docker через установочный файл с офф сайта для платформы запуска) docker build -t currency_app . && docker run --rm -v ${PWD}:/app currency_app Эта команда соберет докер-образ с Debian, создаст контейнер и запустит в нем приложение. Выполнять команду в одной папке с файлом Dockerfile решения

Без Docker (Windows)
3.1 Установить MSYS2: https://www.msys2.org 3.2 Открыть терминал через MSYS2 MinGW 64-bit

3.3 Установить пакеты из консоли: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-curl mingw-w64-x86_64-tinyxml2 mingw-w64-x86_64-libiconv

3.4 скомпилировать программу (там же): g++ main.cpp -o currency_rates -lcurl -ltinyxml2 -liconv -std=c++17

3.5 запустить ее: ./currency_rates.exe
