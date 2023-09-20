- Nombres de los miembros del grupo: Juan Arturo Abaurrea Calafell y Marta González Juan

- Mejoras realizadas: todas.

- Restricciones del programa: el test scripte3.sh tarda 17s en vez de 12s (contando tiempo de compilación).

- Sintaxis específica: Se adjuntará una lista de los posibles ejecutables y su sintaxis.
./mi_mkfs <nombre_dispositivo> <nbloques>
./leer_sf <nombre_dispositivo>
./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>
./leer <nombre_dispositivo> <ninodo>
./permitir <nombre_dispositivo> <ninodo> <permisos>
./truncar <nombre_dispositivo> <ninodo> <nbytes>
./mi_escribir <disco> </ruta_fichero> <texto> <offset>
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
