#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

// clase principal para los materiales
class Material {
protected:
    string cod; // codigo del material
    string tit; // titulo
    int cant_tot; // cantidad en total
    int cant_disp; // cantidad que queda disponible

public:
    // constructor basico
    Material(string c, string t, int tot, int disp) {
        cod = c;
        tit = t;
        cant_tot = tot;
        cant_disp = disp;
    }

    virtual ~Material() {}

    // funcion para mostrar datos en pantalla
    virtual void mostrar() {
        cout << "codigo: " << cod << " | titulo: " << tit;
        cout << " | disp: " << cant_disp << "/" << cant_tot;
    }

    // para obtener el codigo
    string get_cod() {
        return cod;
    }

    // funcion para prestar
    void prestar() {
        if (cant_disp > 0) {
            cant_disp--;
            cout << "prestamo exitoso.\n";
        } else {
            cout << "error: no hay ejemplares disponibles.\n";
        }
    }

    // funcion para devolver
    void devolver() {
        if (cant_disp < cant_tot) {
            cant_disp++;
            cout << "devolucion exitosa.\n";
        } else {
            cout << "error: ya estan todos los ejemplares devueltos.\n";
        }
    }

    // funcion virtual para guardar en archivo
    virtual string guardar_texto() = 0;
};

// clase para los libros que hereda de material
class Libro : public Material {
private:
    string autor;
    int num_pags; // numero de paginas

public:
    Libro(string c, string t, int tot, int disp, string a, int p) : Material(c, t, tot, disp) {
        autor = a;
        num_pags = p;
    }

    void mostrar() override {
        cout << "[libro] ";
        Material::mostrar();
        cout << " | autor: " << autor << " | pags: " << num_pags << "\n";
    }

    string guardar_texto() override {
        // juntamos todo con comas para guardar
        return "L," + cod + "," + tit + "," + to_string(cant_tot) + "," + to_string(cant_disp) + "," + autor + "," + to_string(num_pags);
    }
};

// clase para revistas
class Revista : public Material {
private:
    int num_ed; // numero de edicion
    string mes; // mes de salida

public:
    Revista(string c, string t, int tot, int disp, int ne, string m) : Material(c, t, tot, disp) {
        num_ed = ne;
        mes = m;
    }

    void mostrar() override {
        cout << "[revista] ";
        Material::mostrar();
        cout << " | edicion: " << num_ed << " | mes: " << mes << "\n";
    }

    string guardar_texto() override {
        return "R," + cod + "," + tit + "," + to_string(cant_tot) + "," + to_string(cant_disp) + "," + to_string(num_ed) + "," + mes;
    }
};

int main() {
    // lista para guardar todo
    vector<Material*> lista;
    int opc = 0; // opcion del menu
    string nom_arch = "datos_libreria.txt";

    // intentamos leer el archivo si existe
    ifstream leer_arch(nom_arch);
    if (leer_arch.is_open()) {
        string linea;
        while (getline(leer_arch, linea)) {
            // separamos la linea por las comas
            string datos[7];
            int pos = 0;
            string temp = "";
            for (int i = 0; i < linea.length(); i++) {
                if (linea[i] == ',') {
                    datos[pos] = temp;
                    pos++;
                    temp = "";
                } else {
                    temp += linea[i];
                }
            }
            datos[pos] = temp; // el ultimo pedazo

            if (datos[0] == "L") {
                int tot = stoi(datos[3]);
                int disp = stoi(datos[4]);
                int p = stoi(datos[6]);
                lista.push_back(new Libro(datos[1], datos[2], tot, disp, datos[5], p));
            } else if (datos[0] == "R") {
                int tot = stoi(datos[3]);
                int disp = stoi(datos[4]);
                int ne = stoi(datos[5]);
                lista.push_back(new Revista(datos[1], datos[2], tot, disp, ne, datos[6]));
            }
        }
        leer_arch.close();
    }

    // ciclo del menu
    while (opc != 7) {
        cout << "\n--- menu del sistema ---\n";
        cout << "1. agregar libro\n";
        cout << "2. agregar revista\n";
        cout << "3. mostrar catalogo\n";
        cout << "4. prestar material\n";
        cout << "5. devolver material\n";
        cout << "6. borrar material\n";
        cout << "7. salir\n";
        cout << "ingresa opcion: ";
        cin >> opc;

        // si ponen una letra por error
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "error: pon un numero valido.\n";
            continue;
        }

        if (opc == 1) {
            string c, t, a;
            int tot, p;
            cout << "codigo: ";
            cin >> c;
            cout << "titulo: ";
            cin.ignore();
            getline(cin, t);
            cout << "autor: ";
            getline(cin, a);
            cout << "paginas: ";
            cin >> p;
            cout << "cantidad: ";
            cin >> tot;

            lista.push_back(new Libro(c, t, tot, tot, a, p));
            cout << "libro guardado.\n";

        } else if (opc == 2) {
            string c, t, m;
            int tot, ne;
            cout << "codigo: ";
            cin >> c;
            cout << "titulo: ";
            cin.ignore();
            getline(cin, t);
            cout << "numero edicion: ";
            cin >> ne;
            cout << "mes: ";
            cin >> m;
            cout << "cantidad: ";
            cin >> tot;

            lista.push_back(new Revista(c, t, tot, tot, ne, m));
            cout << "revista guardada.\n";

        } else if (opc == 3) {
            if (lista.empty()) {
                cout << "no hay nada registrado.\n";
            } else {
                for (int i = 0; i < lista.size(); i++) {
                    lista[i]->mostrar();
                }
            }

        } else if (opc == 4) {
            string cod_busc;
            cout << "codigo para prestar: ";
            cin >> cod_busc;
            bool hallado = false;
            for (int i = 0; i < lista.size(); i++) {
                if (lista[i]->get_cod() == cod_busc) {
                    lista[i]->prestar();
                    hallado = true;
                    break;
                }
            }
            if (!hallado) {
                cout << "codigo no existe.\n";
            }

        } else if (opc == 5) {
            string cod_busc;
            cout << "codigo para devolver: ";
            cin >> cod_busc;
            bool hallado = false;
            for (int i = 0; i < lista.size(); i++) {
                if (lista[i]->get_cod() == cod_busc) {
                    lista[i]->devolver();
                    hallado = true;
                    break;
                }
            }
            if (!hallado) {
                cout << "codigo no existe.\n";
            }

        } else if (opc == 6) {
            string cod_busc;
            cout << "codigo para borrar: ";
            cin >> cod_busc;
            bool hallado = false;
            for (int i = 0; i < lista.size(); i++) {
                if (lista[i]->get_cod() == cod_busc) {
                    delete lista[i]; // liberamos memoria
                    lista.erase(lista.begin() + i);
                    cout << "borrado correctamente.\n";
                    hallado = true;
                    break;
                }
            }
            if (!hallado) {
                cout << "codigo no existe.\n";
            }

        } else if (opc == 7) {
            cout << "guardando y saliendo...\n";
            ofstream guardar_arch(nom_arch);
            if (guardar_arch.is_open()) {
                for (int i = 0; i < lista.size(); i++) {
                    guardar_arch << lista[i]->guardar_texto() << "\n";
                }
                guardar_arch.close();
            }
        } else {
            cout << "opcion invalida.\n";
        }
    }

    return 0;
}
