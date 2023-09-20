- Nombres de los miembros del grupo: Juan Arturo Abaurrea Calafell y Marta González Juan

- Mejoras realizadas: Se ha decidido coger la función de liberar_bloques_inodo() más óptima y se ha compactado mediante tres funciones.
La mejora de utilizar la unión en vez del padding en el struct inodo no ha sido realizada porque carecemos de conocimiento sobre las uniones, además de que era voluntario.

- Restricciones del programa: Todavía no es capaz de crear directorios donde se almacenan los ficheros.

- Sintaxis específica: Se adjuntará una lista de los posibles ejecutables y su sintaxis.
./mi_mkfs <nombre_dispositivo> <nbloques>
./leer_sf <nombre_dispositivo>
./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>
./leer <nombre_dispositivo> <ninodo>
./permitir <nombre_dispositivo> <ninodo> <permisos>
./truncar <nombre_dispositivo> <ninodo> <nbytes>