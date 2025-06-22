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
- ** Grafos Direcionados**: Suporte completo a vias de mão única e mão dupla com parsing de tags OSM
- **Visualização de Direção**: Arestas direcionais mostradas em azul com setas, bidirecionais em cinza

##  **Suporte a Grafos Direcionados**


### Tipos de Via Suportados:
- **🔄 Vias Bidirecionais**: Ruas normais de mão dupla (padrão)
- **➡️ Vias de Mão Única**: Ruas com direção única obrigatória  
- **⬅️ Vias Reversas**: Vias de mão única na direção oposta


### Visualização:
- **Arestas Bidirecionais**: Linhas cinzas sem setas
- **Arestas Direcionais**: Linhas azuis com setas indicando direção
- **Caminhos Dijkstra**: Sempre em vermelho, independente do tipo

### Algoritmo:
- **Dijkstra Consciente de Direção**: O algoritmo respeita automaticamente as restrições de direção
- **Busca Otimizada**: Só explora arestas na direção permitida
- **Caminhos Válidos**: Garante que todos os caminhos encontrados respeitam o trânsito

### Visualização Diferenciada:
- **Cores**: Cinza para bidirecionais, azul para mão única
- **Setas**: Setas azuis indicam a direção permitida em vias de mão única
- **Estatísticas**: Interface mostra contagem de vias de cada tipo

## Dependências

- **GTK+ 3.0**: Framework de interface gráfica
- **Cairo**: Biblioteca de gráficos 2D para renderização de grafos
- **libxml2**: Análise XML para arquivos OSM
- **GLib/GIO**: Bibliotecas principais
- **Biblioteca Math**: Funções matemáticas (distância Haversine)
- **Meson**: Sistema de compilação
- **Ninja**: Backend de compilação

## Instalação de Dependências

###  Linux (Ubuntu/Debian)

```bash
# Atualizar repositórios
sudo apt update

# Instalar dependências principais
sudo apt install -y \
    build-essential \
    meson \
    ninja-build \
    pkg-config \
    libgtk-3-dev \
    libcairo2-dev \
    libxml2-dev \
    libglib2.0-dev \
    libgio-2.0-dev

# Verificar instalação
meson --version
ninja --version
pkg-config --modversion gtk+-3.0
```

###  Windows

#### Opção 1: MSYS2 (Recomendado)

1. **Baixar e instalar MSYS2**:
   - Baixe de: https://www.msys2.org/
   - Execute o instalador e siga as instruções

2. **Abrir terminal MSYS2 MINGW64** e instalar dependências:
```bash
# Atualizar pacotes
pacman -Syu

# Instalar toolchain de desenvolvimento
pacman -S --needed \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-meson \
    mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-pkg-config \
    mingw-w64-x86_64-gtk3 \
    mingw-w64-x86_64-cairo \
    mingw-w64-x86_64-libxml2 \
    mingw-w64-x86_64-glib2

# Verificar instalação
meson --version
ninja --version
```

#### Opção 2: Visual Studio + vcpkg

1. **Instalar Visual Studio Community 2022**:
   - Baixe de: https://visualstudio.microsoft.com/pt-br/vs/community/
   - Inclua "Desenvolvimento para Desktop com C++"

2. **Instalar vcpkg**:
```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

3. **Instalar dependências**:
```cmd
.\vcpkg install gtk:x64-windows
.\vcpkg install cairo:x64-windows
.\vcpkg install libxml2:x64-windows
.\vcpkg install glib:x64-windows
```

4. **Instalar Meson e Ninja**:
```cmd
pip install meson ninja
```

###  macOS

```bash
# Instalar Homebrew (se não tiver)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Instalar dependências
brew install \
    meson \
    ninja \
    pkg-config \
    gtk+3 \
    cairo \
    libxml2 \
    glib

# Verificar instalação
meson --version
ninja --version
pkg-config --modversion gtk+-3.0
```

## Compilação

Este projeto usa o sistema de compilação Meson:

### Linux/macOS:
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

### Windows (MSYS2):
```bash
# No terminal MSYS2 MINGW64
meson setup build
ninja -C build
./build/DijikstrasProject.exe
```

### Windows (Visual Studio):
```cmd
# Usando Developer Command Prompt
meson setup build --backend vs
meson compile -C build
.\build\DijikstrasProject.exe
```


## Uso Interativo

### Controles do Mouse
- **Zoom**: Use a roda do mouse para ampliar/reduzir (limitado entre 0.1x e 50x) - centralizado no cursor
- **Panorâmica**: Clique e arraste com o botão esquerdo para mover pelo mapa
- **Seleção de Pontos**: Segure Ctrl e clique nos pontos para selecioná-los para busca de caminhos

### Fluxo de Trabalho da Seleção de Pontos
1. **Carregar Arquivo OSM**: Use Arquivo → Abrir OSM ou botão da barra de ferramentas
2. **Selecionar Ponto de Início**: Segure Ctrl e clique em qualquer ponto (fica verde)
3. **Selecionar Ponto de Fim**: Segure Ctrl e clique em outro ponto (fica vermelho)  
4. **Encontrar Caminho**: Clique no botão "Encontrar Caminho" ou digite os IDs dos pontos manualmente
5. **Resetar Seleção**: Segure Ctrl e clique novamente para começar de novo

### 🔧 **Modos de Edição**

A aplicação agora inclui ferramentas completas de edição de grafos:

#### Modos Disponíveis:
- **🟢 Modo Normal**: Navegação e seleção de pontos para Dijkstra (padrão)
- **➕ Modo Criar**: Clique em espaço vazio para criar novos nós
- **❌ Modo Deletar**: Clique em nós para deletá-los (com validação de conexões)
- **🔗 Modo Conectar**: Clique em dois nós para conectá-los com cálculo automático de distância

#### Como Usar os Modos de Edição:
1. **Ativar Modo**: Clique nos botões da barra de ferramentas (Create Node, Delete Node, Connect Nodes, Normal Mode)
2. **Feedback Visual**: Os nós mudam de cor baseado no modo ativo:
   - Verde claro: Modo criar
   - Vermelho claro: Modo deletar  
   - Azul claro: Modo conectar
   - Azul escuro: Nó selecionado para conexão
   - Laranja: Nó selecionado para edição
3. **Operações**:
   - **Criar**: Clique em qualquer lugar vazio para adicionar um novo nó
   - **Deletar**: Clique em um nó para removê-lo (só permite nós com ≤2 conexões)
   - **Conectar**: Clique no primeiro nó, depois no segundo para criar uma aresta
   - **Normal**: Retorna ao modo padrão de navegação e seleção Dijkstra
-
#### Características Avançadas:
- **Cálculo automático de distância**: Novas conexões usam fórmula Haversine
- **Validação de operações**: Previne deletar nós críticos com muitas conexões
- **IDs automáticos**: Novos nós recebem IDs únicos automaticamente
- **Zoom centrado no mouse**: Zoom mais intuitivo e preciso
- **Visualização completa de caminhos**: Caminhos Dijkstra mostram rota completa em vermelho

### Entrada Manual de Pontos
- Digite o ID do ponto de início no campo "Ponto de Início"
- Digite o ID do ponto de fim no campo "Ponto de Fim"
- Clique em "Encontrar Caminho" para calcular a rota mais curta
- Veja os resultados no painel de resultados


## Estrutura do Projeto

### 🗂️ **Arquitetura Modular**

O projeto foi refatorado para uma **arquitetura modular** que separa responsabilidades e melhora a manutenibilidade:

```
📁 DijkstrasProject/
├── 📄 README.md              # Documentação principal
├── 📄 meson.build            # Configuração de compilação (Meson)
├── 📄 casaPrimo.osm          # Arquivo OSM de exemplo
├── 📁 src/                   # 🔧 Código fonte modular
│   ├── 📄 main.c            # ⚡ Inicialização e configuração 
│   ├── 📄 callbacks.c       # 🎯 Callbacks e eventos GTK 
│   ├── 📄 drawing.c         # 🎨 Renderização Cairo e visualização
│   ├── 📄 interaction.c     # 🖱️ Interação com mouse e teclado
│   ├── 📄 ui_helpers.c      # 🛠️ Funções auxiliares da interface
│   ├── 📄 osm_reader.c      # 📖 Parser de arquivos OSM
│   ├── 📄 dijkstra.c        # 🧮 Algoritmo de caminho mais curto
│   ├── 📄 graph.c           # 📊 Estruturas de dados do grafo
│   └── 📄 edit.c            # ✏️ Funcionalidades de edição
├── 📁 include/               # 📑 Headers modulares
│   ├── 📄 app_data.h        # 🏗️ Estrutura principal AppData
│   ├── 📄 callbacks.h       # 🎯 Declarações de callbacks
│   ├── 📄 drawing.h         # 🎨 API de desenho e renderização
│   ├── 📄 interaction.h     # 🖱️ API de interação
│   ├── 📄 ui_helpers.h      # 🛠️ Utilitários da interface
│   ├── 📄 osm_reader.h      # 📖 API do parser OSM
│   ├── 📄 dijkstra.h        # 🧮 API do algoritmo
│   ├── 📄 graph.h           # 📊 Estruturas do grafo
│   └── 📄 edit.h            # ✏️ API de edição
├── 📁 data/                  # 🎨 Recursos da interface
│   ├── 📄 my_window.ui      # 🖼️ Layout GTK Glade
│   └── 📄 menu.ui           # 📋 Definição de menus
├── 📁 test/                  # 🧪 Programas de teste
│   ├── 📄 run_tests.sh      # 🔄 Script de testes automatizados
│   ├── 📄 test_*.c          # 🔬 Testes unitários específicos
│   └── 📄 verify_fix.sh     # ✅ Verificação de correções
├── 📁 test_data/            # 📋 Dados para testes
│   ├── 📄 simple_test.osm   # 🗺️ Grafo simples para testes
│   └── 📄 test_oneway.osm   # ↗️ Teste de vias direcionais
├── 📁 build/ (ou builddir/) # 🏗️ Arquivos compilados (Meson)
└── 📄 dijkstrasProject.gresource.xml # 📦 Recursos integrados
```

## Características Técnicas

### Algoritmos Implementados:
- **Dijkstra com Min-Heap**: Implementação otimizada com fila de prioridade
- **Haversine**: Cálculo de distância geodésica entre coordenadas
- **Transformação de Coordenadas**: Conversão lat/lon ↔ coordenadas de tela
- **Detecção de Cliques**: Sistema robusto de seleção de pontos próximos

### Tecnologias Utilizadas:
- **C11**: Linguagem de programação principal
- **GTK+ 3.0**: Framework de interface gráfica cross-platform
- **Cairo**: Renderização de gráficos vetoriais 2D
- **libxml2**: Parser XML para arquivos OpenStreetMap
- **Meson + Ninja**: Sistema moderno de compilação

