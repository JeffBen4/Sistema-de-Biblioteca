#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

// =========================================================
// 1. EXCEPCIONES PERSONALIZADAS
// =========================================================
class MaterialNoEncontradoException : public exception {
public:
    const char* what() const throw() override {
        return "ERROR: El material bibliografico solicitado no existe.";
    }
};

class SinStockException : public exception {
public:
    const char* what() const throw() override {
        return "ERROR: Ejemplares agotados. No hay stock disponible para prestamo.";
    }
};

// 👁️ NUEVA EXCEPCIÓN: Para cuando ingresan letras en lugar de números
class EntradaInvalidaException : public exception {
public:
    const char* what() const throw() override {
        return "ERROR: No puede poner texto en esta opcion.";
    }
};

// 👁️ NUEVA EXCEPCIÓN: Para opciones fuera del rango (ej. 8 o -1)
class OpcionMenuInvalidaException : public exception {
public:
    const char* what() const throw() override {
        return "ERROR: Esa opcion no esta disponible en el menu.";
    }
};

// =========================================================
// 2. FUNCIÓN AUXILIAR DE VALIDACIÓN (Evita que el programa se rompa)
// =========================================================
int leerEnteroSeguro() {
    int valor;
    cin >> valor;
    
    // Si el usuario ingresó texto en vez de número, cin.fail() será verdadero
    if (cin.fail()) {
        cin.clear(); // Limpiamos el estado de error de la consola
        cin.ignore(10000, '\n'); // Descartamos la basura que quedó en el buffer
        throw EntradaInvalidaException(); // Lanzamos tu excepción personalizada
    }
    return valor;
}

// =========================================================
// 3. CLASE ABSTRACTA PURA
// =========================================================
class MaterialBibliografico {
protected:
    string codigo;
    string titulo;
    int ejemplaresTotales;
    int ejemplaresDisponibles;

public:
    MaterialBibliografico(string codigo, string titulo, int totales, int disponibles) 
        : codigo(codigo), titulo(titulo), ejemplaresTotales(totales), ejemplaresDisponibles(disponibles) {}

    virtual ~MaterialBibliografico() {}

    virtual void mostrarDetalles() const = 0;
    virtual double calcularPenalidad(int diasRetraso) const = 0;
    virtual string aCadenaCSV() const = 0;

    string getCodigo() const { return codigo; }
    
    void prestarEjemplar() {
        if (ejemplaresDisponibles > 0) ejemplaresDisponibles--;
        else throw SinStockException();
    }
    
    void devolverEjemplar() {
        if (ejemplaresDisponibles < ejemplaresTotales) ejemplaresDisponibles++;
    }

    friend ostream& operator<<(ostream& os, const MaterialBibliografico& mat) {
        os << "[" << mat.codigo << "] " << mat.titulo 
           << " | Disponibles: " << mat.ejemplaresDisponibles << "/" << mat.ejemplaresTotales;
        return os;
    }
};

// =========================================================
// 4. CLASES DERIVADAS
// =========================================================
class Libro : public MaterialBibliografico {
private:
    string autor;
    int numeroPaginas;
public:
    Libro(string c, string t, int tot, int disp, string a, int np)
        : MaterialBibliografico(c, t, tot, disp), autor(a), numeroPaginas(np) {}

    void mostrarDetalles() const override {
        cout << " [LIBRO] " << codigo << " - " << titulo << " | Autor: " << autor 
             << " | Pags: " << numeroPaginas << " | Stock: " << ejemplaresDisponibles << "/" << ejemplaresTotales << endl;
    }

    double calcularPenalidad(int diasRetraso) const override { return diasRetraso * 2.50; }

    string aCadenaCSV() const override {
        return "L," + codigo + "," + titulo + "," + to_string(ejemplaresTotales) + "," + 
               to_string(ejemplaresDisponibles) + "," + autor + "," + to_string(numeroPaginas);
    }
};

class Revista : public MaterialBibliografico {
private:
    int numeroEdicion;
    string mesPublicacion;
public:
    Revista(string c, string t, int tot, int disp, int ne, string m)
        : MaterialBibliografico(c, t, tot, disp), numeroEdicion(ne), mesPublicacion(m) {}

    void mostrarDetalles() const override {
        cout << " [REVISTA] " << codigo << " - " << titulo << " | Edicion: " << numeroEdicion 
             << " | Mes: " << mesPublicacion << " | Stock: " << ejemplaresDisponibles << "/" << ejemplaresTotales << endl;
    }

    double calcularPenalidad(int diasRetraso) const override { return diasRetraso * 1.20; }

    string aCadenaCSV() const override {
        return "R," + codigo + "," + titulo + "," + to_string(ejemplaresTotales) + "," + 
               to_string(ejemplaresDisponibles) + "," + to_string(numeroEdicion) + "," + mesPublicacion;
    }
};

// =========================================================
// 5. PLANTILLA GENÉRICA + STL + MANEJO DE ARCHIVOS
// =========================================================
template <class T>
class Inventario {
private:
    vector<T*> listaElementos;
    string nombreArchivo = "libreria_datos.txt";

public:
    ~Inventario() {
        guardarEnArchivo();
        for (auto& elemento : listaElementos) delete elemento;
        listaElementos.clear();
    }

    void agregar(T* elemento) {
        listaElementos.push_back(elemento);
        cout << "=> Registrado exitosamente.\n";
    }

    void mostrarTodos() const {
        if (listaElementos.empty()) {
            cout << "El inventario esta vacio.\n";
            return;
        }
        for (const auto& elemento : listaElementos) elemento->mostrarDetalles();
    }

    T* buscarPorCodigo(string codigoBuscado) {
        auto it = find_if(listaElementos.begin(), listaElementos.end(),
                          [&codigoBuscado](T* elemento) { return elemento->getCodigo() == codigoBuscado; });
        if (it != listaElementos.end()) return *it;
        throw MaterialNoEncontradoException();
    }

    void registrarPrestamo(string codigo) {
        T* elemento = buscarPorCodigo(codigo);
        elemento->prestarEjemplar();
        cout << "=> Prestamo registrado. Stock restante: " << *elemento << "\n";
    }

    void registrarDevolucion(string codigo) {
        T* elemento = buscarPorCodigo(codigo);
        elemento->devolverEjemplar();
        cout << "=> Devolucion registrada. Stock actual: " << *elemento << "\n";
    }

    void eliminar(string codigo) {
        buscarPorCodigo(codigo);
        auto it = remove_if(listaElementos.begin(), listaElementos.end(),
                            [&codigo](T* elemento) { 
                                if (elemento->getCodigo() == codigo) {
                                    delete elemento; return true;
                                } return false;
                            });
        listaElementos.erase(it, listaElementos.end());
        cout << "=> Material eliminado correctamente.\n";
    }

    void guardarEnArchivo() const {
        ofstream archivo(nombreArchivo);
        if (archivo.is_open()) {
            for (const auto& elemento : listaElementos) {
                archivo << elemento->aCadenaCSV() << "\n";
            }
            archivo.close();
        }
    }

    void cargarDeArchivo() {
        ifstream archivo(nombreArchivo);
        string linea, tipo, cod, tit, autor_mes;
        int tot, disp, pags_ed;

        if (archivo.is_open()) {
            while (getline(archivo, linea)) {
                stringstream ss(linea);
                string token;
                vector<string> datos;
                
                while (getline(ss, token, ',')) datos.push_back(token);
                if (datos.empty()) continue;

                tipo = datos[0]; cod = datos[1]; tit = datos[2];
                tot = stoi(datos[3]); disp = stoi(datos[4]);

                if (tipo == "L") {
                    autor_mes = datos[5]; pags_ed = stoi(datos[6]);
                    listaElementos.push_back(new Libro(cod, tit, tot, disp, autor_mes, pags_ed));
                } else if (tipo == "R") {
                    pags_ed = stoi(datos[5]); autor_mes = datos[6];
                    listaElementos.push_back(new Revista(cod, tit, tot, disp, pags_ed, autor_mes));
                }
            }
            archivo.close();
        }
    }
};

// =========================================================
// 6. BLOQUE PRINCIPAL
// =========================================================
int main() {
    Inventario<MaterialBibliografico> miLibreria;
    miLibreria.cargarDeArchivo();
    
    int opcion = 0; // Inicializamos para evitar basura en memoria

    do {
        cout << "\n=== SISTEMA DE LIBRERIA UCSM ===" << endl;
        cout << "1. Agregar Libro Nuevo" << endl;
        cout << "2. Agregar Revista Nueva" << endl;
        cout << "3. Mostrar Catalogo Completo" << endl;
        cout << "4. Prestar Ejemplar" << endl;
        cout << "5. Devolver Ejemplar" << endl;
        cout << "6. Eliminar Material" << endl;
        cout << "7. Guardar y Salir" << endl;
        cout << "Ingrese opcion: ";

        try {
            // Usamos nuestra función segura en vez de cin >> opcion
            opcion = leerEnteroSeguro();

            // Validamos que no pongan números negativos ni mayores a 7
            if (opcion < 1 || opcion > 7) {
                throw OpcionMenuInvalidaException();
            }

            if (opcion == 1) {
                string cod, tit, aut; int pags, stock;
                cout << "Codigo: "; cin >> cod;
                cout << "Titulo: "; 
                cin >> ws;
                getline(cin, tit);
                cout << "Autor: "; 
                getline(cin, aut);
                
                cout << "Paginas: "; 
                pags = leerEnteroSeguro(); // Protegemos las lecturas internas
                
                cout << "Cantidad de ejemplares que ingresan: "; 
                stock = leerEnteroSeguro();
                
                miLibreria.agregar(new Libro(cod, tit, stock, stock, aut, pags));
            } 
            else if (opcion == 2) {
                string cod, tit, mes; int ed, stock;
                cout << "Codigo: "; cin >> cod;
                cout << "Titulo: "; 
                cin >> ws;
                getline(cin, tit);
                
                cout << "Edicion Nro: "; 
                ed = leerEnteroSeguro();
                
                cout << "Mes: "; cin >> mes;
                
                cout << "Cantidad de ejemplares que ingresan: "; 
                stock = leerEnteroSeguro();
                
                miLibreria.agregar(new Revista(cod, tit, stock, stock, ed, mes));
            }
            else if (opcion == 3) {
                cout << "\n--- CATALOGO ---" << endl;
                miLibreria.mostrarTodos();
            }
            else if (opcion == 4) {
                string cod; cout << "Codigo a prestar: "; cin >> cod;
                miLibreria.registrarPrestamo(cod);
            }
            else if (opcion == 5) {
                string cod; cout << "Codigo a devolver: "; cin >> cod;
                miLibreria.registrarDevolucion(cod);
            }
            else if (opcion == 6) {
                string cod; cout << "Codigo a eliminar: "; cin >> cod;
                miLibreria.eliminar(cod);
            }
        } 
        catch (const exception& e) {
            // Cualquier excepción que hayamos lanzado arriba, cae aquí y no rompe el while
            cout << e.what() << endl;
        }

    } while (opcion != 7);

    cout << "Guardando datos y cerrando el sistema..." << endl;
    return 0;
}
