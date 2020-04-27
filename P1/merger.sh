cantListas=(1024 2048 4096)
tiposInit=('SECUENCIA' 'PRIMOS' 'UNIFORME_CRECIENTE' 'ALEATORIA')
mins=(128)
maxs=(512)

for cant in ${cantListas[*]}
do
    for tipo in ${tiposInit[*]}
    do
        for min in ${mins[*]}
        do
            for max in ${maxs[*]}
            do                
                ./Fuentes/merge $cant $tipo $min $max > medicion.txt
                echo "$(cat medicion.txt) $tipo $cant" >> medicones_tablas
            done
        done
    done
done