# cpp-spreadsheet

Дипломный проект: Электронная таблица

Это упрощённый аналог существующих решений: лист таблицы Microsoft Excel или Google Sheets. В ячейках таблицы могут быть текст или формулы. Формулы, как и в существующих решениях, могут содержать индексы ячеек.

## Инстркукция по установке

Для развертывания проекта понадобятся программы **Cmake, antlr4**

У вас должна получиться следующая структура проекта:

```
spreadsheet/
├── antlr4_runtime/
│   └── Содержимое архива antlr4-cpp-runtime*.zip.
├── build/
├── antlr-4.12.0-complete.jar
├── CMakeLists.txt
├── FindANTLR.cmake
├── Formula.g4
├── Остальные файлы проекта
└── ...
```

В папке build выполняйте генерацию и сборку проекта.</br>
Например при сборке из консоли под windows и IDE MS VisualStudio, нужно набрать такой код:

```
 cmake ../ -DCMAKE_BUILD_TYPE=Debug -DANTLR_BUILD_CPP_TESTS=OFF
 cmake --build . --config=Debug
```

### Установка ANTLR
ANTLR написан на Java, поэтому для его работы вам понадобится комплект разработки [JDK](https://www.oracle.com/java/technologies/downloads/). Вы также можете использовать OpenJDK. Установите JDK в свою систему.
Инструкцию по установке ANTLR можно найти на сайте [antlr.org](https://www.antlr.org/download.html). Более подробные рекомендации представлены в руководстве [Getting Started](https://github.com/antlr/antlr4/blob/master/doc/getting-started.md).

### Использование ANTLR в C++

Чтобы ANTLR сгенерировал для нас исполняемые файлы на С++, нужно выполнить команду:

```
antlr4 -Dlanguage=Cpp Formula.g4
```

Для компиляции полученных файлов понадобится библиотека ANTLR4 C++ Runtime. Скачайте архив antlr4-cpp-runtime*.zip из раздела Download на сайте [antlr.org](https://www.antlr.org/download.html).

### Установка CMake

Перейдите на [страницу скачивания CMake](https://cmake.org/download/) и выберите инсталлятор под вашу архитектуру в разделе Latest Release

