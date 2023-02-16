// inicializar el disco con 10000 bloques inicializados a 0
// bloques físicos =/ bloques lógicos
// 1 mediante argc me dicen nombre dispositivos y cantidad de bloques y si está mal escrito aviso de buena sintaxis
// 2 montar dispositivo (bmount)
// 3 inicializar a 0s (bucle de llamadas a bwrite)
// 4 desmontar dispositivo (bmount)

#include "bloques.h" // solamente para la primera sesión

int main(){
    bmount(argv[1]);
}