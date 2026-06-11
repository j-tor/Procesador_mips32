#include "file_loader.h"
#include <fstream>
#include <iostream>

Fileloader::Fileloader(const std::string& path) : filePath(path) {
    loadfile();
}

void Fileloader::loadfile() {

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    // validación de que el archivo se pueda abrir
    if (!file.is_open()) {
        throw std::runtime_error("Error: No se pudo abrir el archivo en la ruta: " + filePath);
    }

    // validación de que el tamaño sea múltiplo de 4
    std::streamsize fileSize = file.tellg();
    if (fileSize % 4 != 0) {
        file.close();
        throw std::runtime_error("Error: El tamaño del archivo (" + std::to_string(fileSize) + " bytes) no es múltiplo de 4.");
    }

    // volver al inicio del archivo para su lectura
    file.seekg(0, std::ios::beg);

    size_t numInstructions = fileSize / 4;
    
    // vector para almacenar las instrucciones
    instructions.resize(numInstructions);

    // carga en memoria
    if (file.read(reinterpret_cast<char*>(instructions.data()), fileSize)) {
    } else {
        file.close();
        throw std::runtime_error("Error: Ocurrió un problema durante la lectura de los datos del archivo.");
    }

    file.close();
}

std::vector<uint32_t> Fileloader::getInstructiones() const {
    return instructions;
}
