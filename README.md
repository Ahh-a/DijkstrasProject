# Algoritmo de Dijkstra com Visualização de Grafos OSM

Uma implementação completa do algoritmo de caminho mais curto de Dijkstra com suporte a arquivos OpenStreetMap (OSM) e interface gráfica baseada em GTK.

## Características

- **Leitura de Arquivos OSM**: Analisa arquivos XML do OpenStreetMap para extrair nós e caminhos
- **Estruturas de Dados de Grafo**: Armazenamento eficiente de pontos, arestas e pesos
- **Algoritmo de Dijkstra**: Implementação completa com fila de prioridade (min-heap)
- **Interface GTK**: Interface gráfica amigável para carregamento e visualização de arquivos
- **Visualização de Grafos**: Renderização em tempo real do grafo usando Cairo com controles de zoom e panorâmica
- **Seleção Interativa de Pontos**: Clique diretamente nos pontos do mapa usando Ctrl+click para selecionar nós de início/fim
- **Controles do Mouse**: Zoom com roda do mouse, arrastar para navegar, navegação intuitiva
- **Busca de Caminhos**: Cálculo interativo de caminhos entre pontos selecionados com feedback visual
- **Diferenciação Visual**: Pontos selecionados são destacados com cores diferentes (verde=início, vermelho=fim)

## Estrutura do Projeto

```
DijkstrasProject/
├── include/
│   ├── osm_reader.h        # Análise de arquivos OSM e estruturas de grafo
│   ├── graph.h             # Funções de manipulação de grafo
│   └── dijkstra.h          # Implementação do algoritmo de Dijkstra
├── src/
│   ├── main.c              # Aplicação GTK e callbacks da UI
│   ├── osm_reader.c        # Análise XML OSM com libxml2
│   ├── osm_reader_new.c    # Implementação alternativa do analisador OSM
│   ├── graph.c             # Criação e manipulação de grafo
│   └── dijkstra.c          # Algoritmo de caminho mais curto
├── data/
│   ├── my_window.ui        # Layout da interface GTK
│   └── menu.ui             # Menu da aplicação
├── test_data/
│   └── simple_test.osm     # Arquivo OSM de exemplo para testes
├── test/
│   ├── test_events.c       # Teste básico de detecção de eventos
│   ├── test_simple_selection.c # Teste abrangente de seleção de pontos
│   ├── test_selection.sh   # Script de teste automatizado
│   ├── verify_fix.sh       # Script de verificação do projeto
│   ├── test_debug.c        # Programa de depuração
│   ├── test_coordinates.c  # Teste de coordenadas
│   ├── test_draw.c         # Teste de desenho
│   ├── test_locale.c       # Teste de localização
│   ├── test_parsing.c      # Teste de análise
│   ├── test_point_selection.c # Teste de seleção de pontos
│   └── test_app_debug.c    # Depuração da aplicação
├── builddir/               # Diretório de compilação (Meson)
│   └── DijikstrasProject   # Executável compilado
└── casaPrimo.osm          # Arquivo OSM do mundo real para teste
```

## Dependências

- **GTK+ 3.0**: Framework de interface gráfica
- **Cairo**: Biblioteca de gráficos 2D para renderização de grafos
- **libxml2**: Análise XML para arquivos OSM
- **GLib/GIO**: Bibliotecas principais
- **Biblioteca Math**: Funções matemáticas (distância Haversine)

## Compilação

Este projeto usa o sistema de compilação Meson:

```bash
# Configurar compilação
meson setup build

# Compilar
ninja -C build

# Executar
./build/DijikstrasProject

# Alternativa: Executar do builddir
./builddir/DijikstrasProject
```

## Uso Interativo

### Controles do Mouse
- **Zoom**: Use a roda do mouse para ampliar/reduzir (limitado entre 0.1x e 50x)
- **Panorâmica**: Clique e arraste com o botão esquerdo para mover pelo mapa
- **Seleção de Pontos**: Segure Ctrl e clique nos pontos para selecioná-los para busca de caminhos

### Fluxo de Trabalho da Seleção de Pontos
1. **Carregar Arquivo OSM**: Use Arquivo → Abrir OSM ou botão da barra de ferramentas
2. **Selecionar Ponto de Início**: Segure Ctrl e clique em qualquer ponto (fica verde)
3. **Selecionar Ponto de Fim**: Segure Ctrl e clique em outro ponto (fica vermelho)  
4. **Encontrar Caminho**: Clique no botão "Encontrar Caminho" ou digite os IDs dos pontos manualmente
5. **Resetar Seleção**: Segure Ctrl e clique novamente para começar de novo

### Entrada Manual de Pontos
- Digite o ID do ponto de início no campo "Ponto de Início"
- Digite o ID do ponto de fim no campo "Ponto de Fim"
- Clique em "Encontrar Caminho" para calcular a rota mais curta
- Veja os resultados no painel de resultados


## Testes e Depuração

O projeto inclui programas de teste abrangentes no diretório `test/`:

### Programas de Teste
- `test/test_events.c` - Teste básico de detecção de eventos
- `test/test_simple_selection.c` - Teste abrangente de seleção de pontos
- `test/verify_fix.sh` - Verificação automatizada do projeto
- `test/test_debug.c` - Programa de depuração geral
- `test/test_coordinates.c` - Teste de coordenadas
- `test/test_draw.c` - Teste de funções de desenho
- `test/test_locale.c` - Teste de configurações de localização
- `test/test_parsing.c` - Teste de análise de arquivos
- `test/test_point_selection.c` - Teste específico de seleção de pontos
- `test/test_app_debug.c` - Depuração da aplicação principal

### Executando os Testes
```bash
# Executar script de verificação
./test/verify_fix.sh

# Compilar e executar testes individuais
cd test/
gcc `pkg-config --cflags gtk+-3.0` -o test_events test_events.c `pkg-config --libs gtk+-3.0`
./test_events

# Compilar teste de seleção de pontos
gcc -I../include `pkg-config --cflags gtk+-3.0` -o test_simple_selection test_simple_selection.c ../src/osm_reader.c ../src/graph.c ../src/dijkstra.c `pkg-config --libs gtk+-3.0` -lxml2 -lm
./test_simple_selection
```