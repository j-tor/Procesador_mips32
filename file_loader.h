#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

class Fileloader {
public:
    //constructor que recibe la direccion del archivo y y lo lee
    explicit Fileloader(const std::string& path);

    //metodo para obtener el valor de las instrucciones cargadas en memoria
    std::vector<uint32_t> getInstructiones() const;

private:
    std::string filePath;
    std::vector<uint32_t> instructions;

    //metodo para cargar y validar el archivo binario
    void loadfile();
};

#endif // FILE_LOADER_H
