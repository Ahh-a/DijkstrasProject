#!/bin/bash

# Script para compilar e executar testes do projeto Dijkstra
# Execute este script da pasta test/

echo "=== Compilando e executando testes do projeto Dijkstra ==="
echo

# Definir variáveis de compilação
CC="gcc"
CFLAGS="-Wall -Wextra -std=c11 -O2 -g"
INCLUDES="-I../include"
LIBS="`pkg-config --cflags --libs libxml-2.0` -lm"
SRC_DIR="../src"

# Função para compilar e executar um teste
run_test() {
    local test_name=$1
    local test_file="${test_name}.c"
    
    if [ ! -f "$test_file" ]; then
        echo "❌ Arquivo $test_file não encontrado"
        return 1
    fi
    
    echo "🔨 Compilando $test_name..."
    
    # Diferentes configurações de compilação para diferentes testes
    case "$test_name" in
        "test_simple"|"test_casaprimo")
            # Testes simples, apenas OSM reader e graph
            compile_cmd="$CC $CFLAGS $INCLUDES -o \"$test_name\" \"$test_file\" \
                \"$SRC_DIR/osm_reader.c\" \"$SRC_DIR/graph.c\" $LIBS"
            ;;
        *)
            # Testes completos com GTK
            compile_cmd="$CC $CFLAGS $INCLUDES -o \"$test_name\" \"$test_file\" \
                \"$SRC_DIR/osm_reader.c\" \"$SRC_DIR/graph.c\" \"$SRC_DIR/dijkstra.c\" \"$SRC_DIR/edit.c\" \
                $LIBS `pkg-config --cflags --libs gtk+-3.0`"
            ;;
    esac
    
    if eval $compile_cmd; then
        echo "✅ Compilação de $test_name bem-sucedida"
        echo "🚀 Executando $test_name..."
        echo "----------------------------------------"
        ./"$test_name"
        echo "----------------------------------------"
        echo "✅ Teste $test_name concluído"
        echo
        return 0
    else
        echo "❌ Falha na compilação de $test_name"
        echo
        return 1
    fi
}

# Lista de testes a executar
echo "Testes disponíveis:"
echo "1. test_simple - Teste básico de carregamento de OSM"
echo "2. test_casaprimo - Teste completo com casaPrimo.osm"
echo "3. test_directed_graph - Teste de grafos direcionais"
echo "4. test_final_demo - Demonstração final"
echo

# Executar testes específicos ou todos
if [ "$1" = "all" ]; then
    echo "Executando todos os testes..."
    run_test "test_simple"
    run_test "test_casaprimo"
    run_test "test_directed_graph"
    run_test "test_final_demo"
elif [ -n "$1" ]; then
    echo "Executando teste específico: $1"
    run_test "$1"
else
    echo "Uso:"
    echo "  ./run_tests.sh all          - Executar todos os testes"
    echo "  ./run_tests.sh test_simple  - Executar teste específico"
    echo "  ./run_tests.sh              - Mostrar esta ajuda"
fi

echo "=== Testes concluídos ==="
