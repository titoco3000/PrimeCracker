#!/bin/bash

# Função para compilar o programa
compile_program() {
    echo Compilando...

    # garante q 'build' directory existe
    if [ ! -d build ]; then
        mkdir build
    fi
    gcc src/main.c -o build/primeCracker.exe
}

# Default values
compile_flag=false
num_instances=2

# Parse arguments
for arg in "$@"
do
    if [ "$arg" == "-c" ]; then
        compile_flag=true
    elif [[ "$arg" =~ ^[0-9]+$ ]]; then
        num_instances=$arg
    fi
done

# Compila se tem flag -c ou se o exec não existe
if [ "$compile_flag" == true ] || [ ! -f build/primeCracker.exe ]; then
    compile_program
fi

# Abre terminal e executa o programa como lider direto
gnome-terminal -- bash -c "./build/primeCracker.exe -l"

# Espera 300ms para dar tempo de iniciar
sleep 0.3

# Armazena clipboard numa variavel
clipboard_content=$(xclip -o -selection clipboard)

num_to_open=$((num_instances > 8 ? 8 : num_instances))

if [ "$num_to_open" -lt "$num_instances" ]; then
    echo "Número de instâncias foi reduzido para $num_to_open."
fi

# Abre numero especificado de terminais rodando o programa com com clipboard contents como args
for ((i = 0; i < num_to_open; i++))
do
    gnome-terminal -- bash -c "./build/primeCracker.exe \"$clipboard_content\""
done
