## Лабораторная работа 4

Есть устройство, которое публикует по серийному порту или интерфейсу USB текущую температуру окружающей среды. 

Необходимо:
- написать на C\C++ кроссплатформенную программу, которая считывает информацию с порта и записывает ее в лог-файл 
- каждый час программа считает среднюю температуру за час и записывает ее в другой лог-файл
- средняя температура за день записывается в 3й лог-файл 
- лог файл со всеми измерениями должен хранить только измерения за последние 24 часа
- лог-файл со средней температурой за час должен хранить только измерения за последний месяц
- лог файл со средней дневной температурой накапливает информацию за текущий год.

### Windows
```
cd Lab4
./script.cmd
```

### Linux
```
cd Lab4
sudo chmod 777 script.sh
./script.sh
```

### Запуск
Запустите в раздельных сессиях терминала:
```
cd build
./main.exe
```
```
cd build
./simulator.exe
```

### Logfile
Демонстрация вывода 1го лог файла.
```
Sat Jan 18 22:36:19 2025: 7.8518
Sat Jan 18 22:36:20 2025: 19.574
Sat Jan 18 22:36:21 2025: 18.3746
Sat Jan 18 22:36:22 2025: -5.82812
Sat Jan 18 22:36:23 2025: 2.88644
Sat Jan 18 22:36:24 2025: 35.2849
Sat Jan 18 22:36:25 2025: 36.8596
Sat Jan 18 22:36:26 2025: 37.6608
Sat Jan 18 22:36:27 2025: 38.1918
Sat Jan 18 22:36:28 2025: -5.73351
```