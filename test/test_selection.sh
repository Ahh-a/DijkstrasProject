#!/bin/bash

# Test script para verificar seleção de pontos
echo "=== TESTE DE SELEÇÃO DE PONTOS ==="
echo ""
echo "1. Primeiro, vamos carregar o arquivo OSM de teste"
echo "2. Em seguida, vamos testar a seleção de pontos"
echo ""

cd /home/vitorette/Documentos/3semestre/EAD2/DijkstrasProject

# Compilar programa de teste simples
echo "Compilando programa de teste..."
gcc -o test_simple_selection test_simple_selection.c src/osm_reader.c src/graph.c src/dijkstra.c -Iinclude `pkg-config --cflags --libs gtk+-3.0` -lm

if [ $? -eq 0 ]; then
    echo "Compilação bem-sucedida!"
    echo ""
    echo "Executando teste..."
    echo "- Pressione 'o' para carregar arquivo OSM"
    echo "- Use Ctrl+clique para selecionar pontos"
    echo "- Feche a janela para sair"
    echo ""
    ./test_simple_selection
else
    echo "Erro na compilação!"
fi
