- Nombres de los miembros del grupo: Juan Arturo Abaurrea Calafell y Marta González Juan

- Mejoras realizadas: Se ha implementado todas las funciones de niveles inferiores al 10. De este nivel se ha implementado la capacidad de renombrar entradas, moverlas y copiarlas,
aunque estas dos generan problemas en el superbloque. El mi_rm_r se ha empezado pero no se ha conseguido que funcione.

- Restricciones del programa: mi_rm_r no funciona

- Sintaxis específica: Se adjuntará una lista de los posibles ejecutables y su sintaxis.
./mi_mkfs <nombre_dispositivo> <nbloques>
./leer_sf <nombre_dispositivo>
./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>
./leer <nombre_dispositivo> <ninodo>
./permitir <nombre_dispositivo> <ninodo> <permisos>
./truncar <nombre_dispositivo> <ninodo> <nbytes>
./mi_cat <disco> </ruta_fichero>
./mi_chmod <disco> <permisos> </ruta>
./mi_cp_f <disco> </origen/nombre> </destino/>
./mi_cp <disco> </origen/nombre> </destino/>
./mi_link <disco> </ruta_fichero_original> </ruta_enlace>
./mi_ls <disco> </ruta_directorio> <formato>
./mi_mkdir <disco> <permisos> </ruta>
./mi_mv <disco> </origen/nombre> </destino/>
./mi_rm_r <disco> </ruta>
./mi_rm <disco> </ruta>
./mi_rn <disco> </ruta/antiguo> <nuevo>
./mi_stat <disco> </ruta>
./mi_touch <disco> <permisos> </ruta>
