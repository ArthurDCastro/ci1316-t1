#!/bin/bash
echo "USAGE: ./rodaTodos.sh <nElements>"
    
# for i in {1..8}
# do
#     echo "Executando 10 vezes o bsearch_function_pa com $1 elementos e $i threads:"
#     for j in {1..10}
#     do
#         ./bsearch_function_pa $1 $i | grep -oP '(?<=Tempo total em segundos: )[^ ]*'
#     done
# done

for i in {1..8}
do
    echo "Executando 10 vezes o bsearch_function_pb com $1 elementos e $i threads:"
    for j in {1..10}
    do
        ./bsearch_function_pb $1 $i | grep -oP '(?<=Tempo total em segundos: )[^ ]*'
    done
done
