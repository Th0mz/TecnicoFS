## Tecnico FS
---
### Objetivo

O objetivo final do projeto é desenvolver um File System em user-level e que mantém os seus conteúdos em memória primária, chamado TecnicoFS.

Os FS em modo utilizador são uma alternativa que tem vindo a ganhar relevância recentemente, já que permitem o desenvolvimento rápido de FS facilmente portáveis e comforte isolamento de falhas.

Num FS em modo utilizador, as funcionalidades do FS são oferecidas num processo servidor (que, naturalmente, corre em modo utilizador). Outros processos podem chamar funções do FS através de pedidos ao núcleo do Sistema Operativo, que, por sua vez, encaminha esses pedidos ao processo servidor do FS através de um canal de comunicação estabelecido com este. Posteriormente, o retorno da função é devolvido ao cliente invocado pela via inversa.

---
### Etapas de desenvolvimento
#### Exercício 1:
 Desenvolve o servidor do TecnicoFS, composto por uma pool de tarefas escravas que executam as operações do FS sobre a diretoria partilhada em memória.

Neste exercício foram implementadas as seguntes funcionalidades:

 - **Carregamento do ficheiro de entrada**
<br> Ler os comandos do `inputfile` de forma a serem executados posteriormente.

 - **Paralelização do servidor**    
    - <u>Pool de tarefas:</u> Criação de uma pool de tarefas que vão executar os comandos lidos. 
    - <u>Sincronização por trinco global:</u> Dado que estamos a trabalhar sobre memória partilhada devemos sincronizar as secções criticas do código de forma a não haver erros de memória.

 - **Terminação do servidor**
<br> Terminar o servidor (todas as tarefas criadas) e dar display do tempo que execução.

 Modo de execução :

    ./tecnicofs inputfile outputfile numthreads synchstrategy

#### Exercício 2:
 Estende a solução anterior com uma sincronização mais fina, o que permitirá maior paralelismo efetivo. Também melhora o carregamento do ficheiro de entrada para permitir que as operações se executem em paralelo com o carregamento inicial do ficheiro. Finalmente, estende a API do FS com a operação de renomear ficheiro, com uma semântica atómica.

Neste exercício foram implementadas as seguntes funcionalidades:

- **Sincronização fina dos inodes**
<br> Em vez de usar um trinco global, passamos a usar um trinco fino, de forma a tornar mais eficiente a execução de comandos que apenas leem informação. 

- **Execução incremental de comandos**
<br> Em vez de carregar e executar comandos como no primeiro exercício, agora temos uma tarefa que carrega comandos do ficheiro e outras que executam esses comandos em simultâneo.

- **Nova operação: mover ficheiro ou diretoria**
<br> Função de mover ficheiros/diretorias implementada de raiz.

- **Shell script**
<br> Script para avaliar o desempenho do programa TecnicoFS, quando executado com diferentes ficheiros de entrada.

#### Exercício 3:
Estende a solução anterior suportando a invocação de operações do FS por processos cliente. Os processos cliente fazem chamadas ao TecnicoFS enviando mensagens através de um socket, que são consumidas pelas tarefas no servidor e que respondem aos clientes com o respetivo resultado. Adicionalmente, implementa as operações em falta da API do TecnicoFS.

Neste exercício foram implementadas as seguntes funcionalidades:

-  **Comunicação com processos cliente**
<br> O servidor TecnicoFS deixa de carregar comandos a partir de um ficheiro. Em vez disso, passa a ter um socket Unix do tipo datagram, através do qual recebe (e responde a) pedidos de operações solicitadas por outros processos, que designamos de processos cliente.

-  **Nova operação ‘p’**
<br> Implementação de mais uma operação sobre TecnicoFS, que recebe como argumento um ficheiro e escreve todo o conteúdo do TecnicoFS para esse ficheiro de output.
---

### Comandos
| Comando | Funcionalidade |
| :-----: | ------- |
| c `nome` `tipoo`| Adiciona à diretoria uma nova entrada, cujo nome e tipo (diretoria ou ficheiro) são indicados em argumento. O tipo pode ser ‘d’ (diretoria) ou ‘f’ (ficheiro normal) |
| l `nome` | Pesquisa o TecnicoFS por um ficheiro/diretoria com nome indicado em argumento|
| d `nome` | Apaga do TecnicoFS o ficheiro/diretoria com o nome indicado em argumento. |
| m `origem` `destino` | Move o ficheiro da origem para o destino |
| p `outputFile` | Imprime o conteúdo atual do TecnicoFS para um ficheiro de saída indicado como único argumento da operação |

<br>
Exemplos : 

    c /a f -> cria o ficheiro a na root
    c /b d -> cria a diretoria b na root
    c /c f -> cria o ficheiro c na root 
    l /a -> verifica que o inode com path /a existe
    d /a -> remove o inode a
    m /c /b/c -> move o ficheiro c da root para dentro da diretoria /b
    
---