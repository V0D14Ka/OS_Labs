## Лабораторная работа 5

В программу, выполненную в ходе предыдущего задания, добавить функционал сетевого сервера с возможностью публиковать по HTTP-запросу текущую температуру, а также статистику за выбранный период времени. Добавить функционал сохранения логов в базу данных вместо файлов.
Написать для своего сервера клиентское веб-приложение на любом языке программирования (C/C++ + HTML + CGI, PHP, Python, Java...), отображающее данные в наглядном виде (таблицы или графики).

## Реализация
- В качестве СУБД - SQLite.
- В качестве WEB - Flask.

## Установка

### Windows
```
cd Lab5
./script.cmd
```

### Linux
```
cd Lab5
sudo chmod 777 script.sh
./script.sh
```

## Использование

Запустите все необходимые программы в разных сессиях терминала:

- Эмулятор МК:
    ```
    cd build
    ./simulator.exe
    ```
- Основная программа, вся логика обработки температур здесь:
    ```
    cd build
    ./main.exe
    ```
- HTTP сервер:
    ```
    cd build
    ./server.exe
    ```
- WEB:
    - Windows:
        ```
        ./start_web.cmd
        ```
    - Linux:
        ```
        ./start_web.sh
        ```

Сервер **http://localhost:8080/**
Эндпоинты сервера:
- GET *temperature* - получает последнюю зафиксированную (текущую) температуру.
- GET | POST *history* - получает историю за выбранный период времени. 
    
Веб-морда **http://localhost:5000/**