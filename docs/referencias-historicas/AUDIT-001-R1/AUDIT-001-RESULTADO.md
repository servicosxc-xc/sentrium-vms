# AUDIT-001 — resultado

**GO WITH BLOCKERS**, exclusivamente para revisão e eventual autorização posterior de CPP-001. Nenhuma Foundation C++ foi iniciada. Auditoria estática; produto, builds, testes, serviços, SDK e modelos não foram executados. Não houve acesso ao DataRoot/banco operacional nem alteração intencional de r24/r25.

## Escopo e evidência

- Árvore r25: 1922 arquivos no manifesto inicial e 1922 no final, incluindo gerados/binários/logs; sem exclusão seletiva de artefatos.
- Inventário: 377 arquivos de fontes/recursos, 13 projetos (8 produto, 3 laboratórios, 2 testes), 63.446 linhas C# físicas; 36.043 pertencem ao wrapper NetSDK, 27.403 às demais fontes. Símbolos inventariados são correspondências lexicais, não análise AST/compilação.
- Matriz: 377 componentes por arquivo com IDs estáveis; funções futuras separadas. 151 casos existentes mapeados com localização e equivalente C++; Passed refere-se aos relatórios anteriores, não a nova execução.
- API: 75 mapeamentos de métodos HTTP, além de WebSocket mídia e hub SignalR. Persistência: cinco tabelas SQLite e oito índices explícitos; JSON externo continua relevante.
- SDK: 10 métodos NETClient realmente referenciados fora do wrapper, mais PlaySDK/Win32; 242 declarações P/Invoke não equivalem a 242 APIs necessárias em produção. Dependências PE foram lidas, nunca carregadas.

## Integridade r25

1 divergência(s) no manifesto integral; não é possível declarar a árvore inteira intacta. Fontes inventariadas alteradas: 0. Ver comparação e explicação abaixo.

- `src/XC.VideoIA.Server/bin/Debug/net10.0/sdk_log/sdk_log.log`: changed.

A auditoria não escreveu na r25 e não parou processos existentes. Um log SDK aberto por outro processo foi observado durante a leitura. Atribuição a processo externo é compatível com esse fato, mas o hash sozinho não prova autoria; a divergência não será ocultada.

[Comparação completa](artifacts/R25-COMPARACAO-SHA256.json), [antes](artifacts/R25-SHA256-ANTES.json), [depois](artifacts/R25-SHA256-DEPOIS.json). A comparação verifica conteúdo/path/tamanho; não pretende provar metadados ACL/timestamps nem estado de outro DataRoot.

## Respostas para decisão

| Pergunta | Resposta / documento |
|---|---|
| 1. O que existe? | [Auditoria](docs/AUDITORIA-R25-PARA-CPP.md) e inventário JSON: aplicações servidor/WPF, SDK, IA, Python LPR, eventos, evidências, integração, testes e labs |
| 2. O que deve existir em v2? | paridade útil e pendências explícitas em [matriz](docs/MATRIZ-R25-PARA-CPP.md); listas/usuários/gravação não dados como prontos |
| 3. O que portar? | regras puras, normalização, contratos e casos de teste, conforme classificação por arquivo |
| 4. O que redesenhar? | concorrência, ownership, storage/security, filas, render, ações e integração |
| 5. O que substituir? | WPF/.NET e bridge Python por implementações nativas somente após goldens |
| 6. UI? | Qt Quick proposto sob benchmark/licença; Widgets/render nativo e WinUI comparados em [UI](docs/UI-PARIDADE-R25-PARA-CPP.md) |
| 7. Drogon? | candidato adequado ao control plane; inferência/decode/storage pesado fora do HTTP loop; SignalR exige compatibilidade específica |
| 8. FFmpeg? | candidato demux/decode após prova RAW/DHAV; PlaySDK permanece oracle de comparação |
| 9. Hardware decode? | D3D11VA prioritário no laboratório Windows; software fallback; NVDEC/QSV opcionais |
| 10. IA/LPR nativos? | ORT C++ e pipelines fiéis; OCR observado por slots/alphabet, não CTC presumido |
| 11. Intelbras faltante? | NetSDK/PlaySDK C++ x64 e libt2u nativa identificados; faltam licença libt2u, bootstrap oficial, confirmação standalone, Windows limpo, reconexão/failover e compatibilidade atual |
| 12. SQLite? | preservar schema/JSON/UTC; RAII e concorrência explícita; sem abrir banco operacional |
| 13. DataRoot? | raiz v2-dev isolada; futura importação offline autorizada por cópia/backup; DataProtection não é DPAPI bruto |
| 14. Paridade? | corpus golden em raízes isoladas, comparações JSON/SQLite/frame/evento e UX |
| 15. 151 testes? | [mapa completo](docs/MAPA-TESTES-R25-PARA-CPP.md), sem afirmar que substituem testes nativos/soak |
| 16. Licenças? | [dependências](docs/DEPENDENCIAS-E-LICENCAS.md): Qt/módulos GPL, FFmpeg build/patentes, SDK, drivers e pesos |
| 17. Estrutura CPP-001? | árvore proposta em [arquitetura](docs/ARQUITETURA-CPP-PROPOSTA.md); nenhum arquivo C++/CMake de produto criado |
| 18. Ordem? | [roadmap](docs/ROADMAP-CPP.md), segurança/contratos/storage antes da migração de funcionalidades |
| 19. Bloqueios? | [riscos](docs/RISCOS-E-BLOQUEIOS.md): IB-01..06 da integração Cloud em produção, modelos/YAML/licenças, formato mídia, SignalR/segredos, Official e hardware |
| 20. Prova de melhora? | A/B/C/D, entradas iguais, CPU/GPU/cópias/p95/p99/latência/qualidade e soak com CSV bruto; nenhuma performance declarada resolvida |

## Achados que não devem desaparecer na migração

Automático já promove o tile maximizado para Principal; modos fixos não mudam. Menus de áudio/talk/gravação/instant-playback e snapshot contextual contêm placeholders, embora snapshot de evidência e DAV tenham implementação separada. Favoritos de sessão não provam persistência. Usuários/listas completos, atributos veiculares adicionais e homologação Official são backlog. Unknown da outbox não admite retry cego; evidências exigem HTTPS, opaque ID, TTL, auth, auditoria e retenção protegida. Ações são por regra/perfil.

## Entrega e limites

15 documentos obrigatórios, relatórios suplementares e 10 ADRs **Proposed**. [Validação](artifacts/VALIDACAO-AUDITORIA.json) registra cobertura, links e empacotamento. Os scripts são ferramentas estáticas da auditoria, não implementação v2. ZIP contém somente esta auditoria; SHA256 externo acompanha o ZIP. Não contém árvore r25, bancos, DataRoot, DLLs, modelos ou credenciais operacionais exportadas. Inspeção automatizada de padrões não é prova universal de ausência de segredo; configurações foram inventariadas por nomes de chaves, sem exportar seus valores.

**PARAR AQUI.** A próxima Foundation depende de revisão e instrução do usuário.

## Revisão documental Intelbras — 23/10/2026

[Addendum](AUDIT-001-INTELBRAS-CLOUD-ADDENDUM.md): remove indisponibilidade C++ e obrigação .NET como blockers; separa xc_cloud/xc_intelbras e atualiza IB-01..06. GO WITH BLOCKERS mantido para Foundation desacoplada futura; integração Cloud ainda não pronta para produção. Evidências fornecidas pelo usuário, sem repetir capturas/probes nesta revisão. Manifestos históricos antes/depois e a exceção original do log foram preservados. A comparação específica desta revisão consta de [validação da revisão](artifacts/VALIDACAO-REVISAO-INTELBRAS.json). CPP-001 NÃO foi iniciada.