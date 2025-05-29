#include "fs.h"
#include <Arduino.h>
#include "fs/logger.h"
#include "kernel/scheduler.h"

extern Logger logger;
extern Scheduler kernel;
FileSystem fs; 

/**
 * @brief Конструктор файловой системы
 */
FileSystem::FileSystem() 
{
    for (int i = 0; i < MAX_FILES; i++) 
    {
        files[i].data = nullptr;
        files[i].size = 0;
    }
}

/**
 * @brief Деструктор файловой системы
 */ 
FileSystem::~FileSystem() 
{
    for (int i = 0; i < fileCount; i++) 
    {
        freeFileData(i);
    }
}

/**
 * @brief Начало операции с ФС (блокировка)
 * @return true если операция разрешена, false если ФС занята
 */
bool FileSystem::beginOperation() {
    if(_busy) 
    {
        logger.log("FS: Operation rejected (busy)");
        return false;
    }
    _busy = true;
    return true;
}

/**
 * @brief Завершение операции с ФС (разблокировка)
 */
void FileSystem::endOperation() 
{
    _busy = false;
}

/**
 * @brief Проверка корректности имени файла
 * @param name Имя файла для проверки
 * @return true если имя корректно
 */
bool FileSystem::validateFilename(const String& name) 
{
    return name.length() > 0 && 
           name.length() <= MAX_FILENAME_LEN &&
           name.indexOf('/') == -1;
}

/**
 * @brief Проверка допустимого размера файла
 * @param size Размер для проверки
 * @return true если размер допустим
 */
bool FileSystem::validateSize(size_t size) 
{
    return size <= MAX_FILE_SIZE;
}

/**
 * @brief Проверка целостности файловой системы
 * @return true если ФС в корректном состоянии
 */
bool FileSystem::verifyFilesystem() 
{
    if(!beginOperation()) return false;
    
    bool valid = true;
    for(int i = 0; i < fileCount; i++) 
    {
        if(!validateFilename(files[i].name) || !validateSize(files[i].size)) 
        {
            valid = false;
            break;
        }
    }
    
    endOperation();
    return valid;
}

/**
 * @brief Поиск индекса файла по имени
 * @param name Имя файла для поиска
 * @return Индекс файла или -1 если не найден
 */
int FileSystem::findFileIndex(const String& name) 
{
    for (int i = 0; i < fileCount; i++) 
    {
        if (files[i].name == name) 
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Освобождение памяти файла
 * @param index Индекс файла
 */
void FileSystem::freeFileData(int index) 
{
    if (files[index].data != nullptr) 
    {
        delete[] files[index].data;
        files[index].data = nullptr;
    }
}

/**
 * @brief Создание текстового файла
 * @param name Имя файла
 * @param content Содержимое файла
 * @return true если файл создан успешно
 */
bool FileSystem::createFile(const String& name, const String& content) 
{
    if(!validateFilename(name)) 
    {
        logger.log("ERR: Invalid filename");
        return false;
    }
    
    if(!validateSize(content.length()))
    {
        logger.log("ERR: File too big");
        return false;
    }
    
    if(fileCount >= MAX_FILES) 
    {
        logger.log("ERR: Max files reached");
        return false;
    }
    
    int index = findFileIndex(name);
    if(index != -1) 
    {
        logger.log("ERR: File exists");
        return false;
    }

    files[fileCount].name = name;
    files[fileCount].isBinary = false;
    files[fileCount].data = new uint8_t[content.length() + 1];
        if (files[fileCount].data == nullptr) 
        {
        logger.log("ERR: Memory allocation failed");
        kernel.emergencyDump("Memory allocation failed"); 
        return false;
        }
    memcpy(files[fileCount].data, content.c_str(), content.length() + 1);
    files[fileCount].size = content.length() + 1;
    
    fileCount++;
    return true;
}

/**
 * @brief Создание бинарного файла
 * @param name Имя файла
 * @param data Указатель на данные
 * @param size Размер данных
 * @return true если файл создан успешно
 */
bool FileSystem::createBinaryFile(const String& name, const uint8_t* data, size_t size)
 {
    if (fileCount >= MAX_FILES || size > MAX_FILE_SIZE) return false;
    
    int index = findFileIndex(name);
    if (index != -1) return false;

    files[fileCount].name = name;
    files[fileCount].isBinary = true;
    files[fileCount].data = new uint8_t[size];
    memcpy(files[fileCount].data, data, size);
    files[fileCount].size = size;
    
    fileCount++;
    return true;
}

/**
 * @brief Чтение текстового файла
 * @param name Имя файла
 * @return Содержимое файла или пустая строка при ошибке
 */
String FileSystem::readFile(const String& name) 
{
    int index = findFileIndex(name);
    if (index == -1 || files[index].isBinary) return "";

    return String((char*)files[index].data);
}

/**
 * @brief Чтение бинарного файла
 * @param name Имя файла
 * @param buffer Буфер для данных
 * @param bufferSize Размер буфера
 * @return true если чтение успешно
 */
bool FileSystem::readBinaryFile(const String& name, uint8_t* buffer, size_t bufferSize) 
{
    int index = findFileIndex(name);
    if (index == -1 || !files[index].isBinary || bufferSize < files[index].size) 
    {
        return false;
    }

    memcpy(buffer, files[index].data, files[index].size);
    return true;
}

/**
 * @brief Запись в текстовый файл
 * @param name Имя файла
 * @param content Содержимое для записи
 * @return true если запись успешна
 */
bool FileSystem::writeFile(const String& name, const String& content) 
{
    int index = findFileIndex(name);
    if (index == -1) {
        return createFile(name, content);
    }

    freeFileData(index);
    files[index].isBinary = false;
    files[index].data = new uint8_t[content.length() + 1];
    memcpy(files[index].data, content.c_str(), content.length() + 1);
    files[index].size = content.length() + 1;
    return true;
}

/**
 * @brief Запись в бинарный файл
 * @param name Имя файла
 * @param data Указатель на данные
 * @param size Размер данных
 * @return true если запись успешна
 */
bool FileSystem::writeBinaryFile(const String& name, const uint8_t* data, size_t size) 
{
    int index = findFileIndex(name);
    if (index == -1) {
        return createBinaryFile(name, data, size);
    }

    freeFileData(index);
    files[index].isBinary = true;
    files[index].data = new uint8_t[size];
    memcpy(files[index].data, data, size);
    files[index].size = size;
    return true;
}

/**
 * @brief Удаление файла
 * @param name Имя файла
 * @return true если удаление успешно
 */
bool FileSystem::deleteFile(const String& name) 
{
    int index = findFileIndex(name);
    if(index == -1) {
        logger.log("ERR: File not found");
        return false;
    }

    freeFileData(index);
    // Сдвигаем массив файлов
    for (int i = index; i < fileCount - 1; i++) 
    {
        files[i] = files[i + 1];
    }
    fileCount--;
    return true;
}

/**
 * @brief Проверка существования файла
 * @param name Имя файла
 * @return true если файл существует
 */
bool FileSystem::fileExists(const String& name) 
{
    return findFileIndex(name) != -1;
}

/**
 * @brief Получение списка файлов
 * @return Форматированная строка с информацией о файлах
 */
String FileSystem::listFiles() 
{
    String result;
    
    for(int i = 0; i < fileCount; i++) 
    {
        result += "  ";
        result += files[i].name;
        result += " (";
        result += files[i].isBinary ? "binary" : "text";
        result += ", ";
        result += files[i].size;
        result += " bytes)\n";
    }
    return result;
}