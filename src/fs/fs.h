#ifndef FS_H
#define FS_H

#include <Arduino.h>

class FileSystem 
{
public:
    struct File 
    {
        String name;     
        uint8_t* data;     
        size_t size;       
        bool isBinary;     
    };

    static const int MAX_FILES = 5;         
    static const int MAX_FILE_SIZE = 512;    
    static const int MAX_FILENAME_LEN = 16;  

    FileSystem();
    ~FileSystem();
    
    bool verifyFilesystem();
    
    bool createFile(const String& name, const String& content = "");
    bool createBinaryFile(const String& name, const uint8_t* data, size_t size);
    String readFile(const String& name);
    bool readBinaryFile(const String& name, uint8_t* buffer, size_t bufferSize);
    bool writeFile(const String& name, const String& content);
    bool writeBinaryFile(const String& name, const uint8_t* data, size_t size);
    bool deleteFile(const String& name);
    bool fileExists(const String& name);
    String listFiles();
    
    int getFileCount() const { return fileCount; }

private:
    File files[MAX_FILES]; 
    int fileCount = 0;      
    volatile bool _busy = false;

    int findFileIndex(const String& name);
    void freeFileData(int index);
    bool beginOperation();
    void endOperation();
    bool validateFilename(const String& name);
    bool validateSize(size_t size);
};

extern FileSystem fs; 

#endif