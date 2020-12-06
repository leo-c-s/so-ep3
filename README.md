# Arquivo
tamanho máximo de nome de arquivo: 128

## Metadados:
- indice do nome: 4 bytes
- data de criação: 4 bytes
- data de última modificação: 4 bytes
- data de último acesso: 4 bytes
- indice do primeiro bloco: 4 bytes
- tamanho: 4 bytes
- tipo (arquivo normal ou diretório): 4 bytes

tamanho total dos metadados: 28 bytes  
datas armazenadas em segundos desde a Epoch como inteiros  
endereço do primeiro bloco é inteiro  

# Diretório
- quantidade máxima de arquivos: 128
- espaço total para metadados: 3584 bytes -> 1 bloco
- espaço total para nomes de arquivos: 16384 bytes -> 5 blocos
- tamanho total de diretório: 6 blocos
