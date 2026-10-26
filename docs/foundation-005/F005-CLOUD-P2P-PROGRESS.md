# SENTRIUM VMS — PROGRESSO CLOUD / P2P

**Produto:** SENTRIUM VMS — Intelligent Video Management System  
**Área:** Conectividade de dispositivos  
**Foundation relacionada:** Foundation-005 — Device Connectivity  
**Data:** 25/10/2026  
**Estado:** **DOCUMENTO DE PROGRESSO — SEM AUTORIZAÇÃO DE IMPLEMENTAÇÃO**

---

## 1. Objetivo deste documento

Consolidar o que já foi aprendido, decidido e planejado para o suporte **Cloud/P2P** do SENTRIUM VMS.

Este documento não autoriza implementação da Foundation-005 e não altera o estado atual da F004. Ele serve como referência técnica e de continuidade.

---

## 2. Estado atual

O SENTRIUM já possui conectividade funcional por **LAN / IP direto**, validada na Foundation-003 com NetSDK real.

O Cloud/P2P ainda **não foi implementado na nova base C++**.

A próxima Foundation de conectividade deverá evoluir o cliente para suportar três estratégias de acesso ao mesmo dispositivo lógico:

1. LAN / IP direto;
2. Cloud / P2P;
3. Registro Automático.

O objetivo é que o restante do VMS consuma uma sessão de dispositivo sem precisar conhecer os detalhes do transporte usado para obtê-la.

---

## 3. Conhecimento recuperado do legado

O legado XC Video IA / SIM Next demonstrou um caminho Cloud/P2P funcional baseado na cadeia:

`SIMNext.P2PSDK.dll → Libt2uWrapper.dll → libt2u.dll`

Esse caminho não deve ser copiado integralmente para o C++.

A orientação é reutilizar somente o **subset mínimo necessário**, após auditar novamente os binários, exports, contratos e comportamento real.

Pontos importantes observados no legado:

- Cloud/P2P cria um túnel/sessão de transporte para alcançar o equipamento;
- a vida útil do túnel não deve ser confundida com a vida útil do RealPlay;
- login NetSDK, RealPlay e mídia continuam sendo responsabilidades separadas;
- P2P direto e Relay possuem comportamento e latência diferentes;
- o legado distinguia conexão direta de conexão por Relay por um estado/proxy retornado pela camada P2P;
- a semântica histórica a revalidar é: `proxy == 1` indicando Relay e conexão estabelecida sem proxy indicando P2P direto.

Essa semântica só deve entrar em produção depois de ser novamente comprovada no kit/binários usados pelo SENTRIUM.

---

## 4. Decisão de arquitetura para a nova geração

O SENTRIUM não terá duas árvores, dois dispositivos ou dois pipelines de vídeo para LAN e Cloud.

O mesmo dispositivo lógico deverá possuir:

- DeviceId;
- estado de conexão;
- transporte atual;
- sessão NetSDK utilizável;
- quantidade real de canais;
- connect/disconnect/reconnect;
- diagnóstico sanitizado.

O transporte será uma propriedade da sessão, não a identidade do dispositivo.

Exemplo conceitual:

```
Dispositivo
 ├─ identidade lógica
 ├─ credenciais/configuração
 ├─ transporte
 │   ├─ LAN
 │   ├─ P2P direto
 │   ├─ Relay
 │   └─ Registro Automático
 └─ sessão NetSDK
     ├─ canais
     ├─ RealPlay
     └─ demais operações
```

O pipeline de mídia permanece independente do mecanismo que entregou a sessão.

---

## 5. Interface planejada

Na interface do SENTRIUM, o fluxo de **Adicionar dispositivo** deverá permitir escolher o tipo de conexão.

### LAN

Campos já conhecidos:

- nome;
- IP/host;
- porta;
- usuário;
- senha.

### Cloud / P2P

A interface deverá receber somente os dados realmente exigidos pelo SDK/serviço depois da auditoria.

Não serão inventados campos antes da validação do contrato real.

A UI deverá indicar o tipo de transporte obtido após a conexão.

Semântica visual planejada:

- **azul:** conexão P2P direta;
- **amarelo:** conexão via Relay.

Essas cores representam **transporte**, e não estado genérico da câmera.

As câmeras permanecem neutras quando não estão sendo visualizadas e usam indicador verde somente quando estão abertas/ativas em visualização.

---

## 6. Separação de responsabilidades

A arquitetura futura deverá manter claramente separadas as seguintes etapas:

### 6.1 Descoberta / estabelecimento Cloud

Responsável por localizar o equipamento e estabelecer P2P direto ou Relay.

### 6.2 Sessão de dispositivo

Responsável pelo login e estado NetSDK.

### 6.3 RealPlay / mídia

Responsável por Principal/Extra, ingest, decode e render.

### 6.4 Reconexão

A perda do RealPlay não significa necessariamente perda do túnel Cloud.

Da mesma forma, uma mudança de P2P direto para Relay não deve criar um novo dispositivo lógico.

Essa separação é importante para evitar o acoplamento observado em soluções que tratam conexão Cloud e reprodução como uma única operação.

---

## 7. Relação com a F004

A F004 já comprovou que, quando uma sessão NetSDK válida existe, o pipeline de mídia consegue trabalhar com:

- NetSDK RAW/DHAV;
- PlaySDK;
- FFmpeg DHAV demux;
- H.264 software decode;
- D3D11VA.

Portanto a Foundation-005 não precisa reinventar a mídia.

O objetivo da camada Cloud é entregar uma sessão utilizável para o mesmo pipeline já validado.

Isso reduz risco arquitetural: Cloud/P2P fica concentrado em conectividade, enquanto mídia permanece uma responsabilidade separada.

---

## 8. Por que Cloud passou a ser prioridade

O NVR LAN usado atualmente no laboratório não fornece quantidade suficiente de câmeras ativas para representar um benchmark de 16/32 canais.

Ao implementar Cloud/P2P e Registro Automático primeiro, o SENTRIUM poderá acessar gravadores completos disponíveis em outros locais e então validar:

- 16/32 canais reais;
- Principal e Extra;
- P2P direto versus Relay;
- ingest;
- decode;
- render;
- CPU/GPU/memória;
- reconexão;
- estabilidade prolongada.

Essa mudança de sequência evita criar benchmark artificial com canais vazios.

---

## 9. Pontos já considerados bloqueadores de produção

Antes de considerar Cloud pronto para produção, ainda precisam ser fechados:

### IB-01 — Direito de redistribuição
Confirmar licença e condições para distribuir os binários necessários junto ao SENTRIUM.

### IB-02 — Bootstrap
Entender exatamente quais arquivos, configurações ou serviços a camada P2P precisa para inicializar em uma instalação limpa.

### IB-03 — Independência do SIM Next
Comprovar funcionamento sem depender de instalação prévia, registro ou estado residual do SIM Next.

### IB-04 — Clean Windows
Validar em Windows limpo, com somente os componentes autorizados do SENTRIUM.

### IB-05 — P2P direto ↔ Relay
Comprovar detecção correta do modo de transporte e comportamento durante transições/reconexões.

### IB-06 — Compatibilidade
Validar modelos/firmwares/equipamentos suportados e comportamento com NVRs reais de diferentes capacidades.

Nenhum desses blockers deve ser considerado resolvido somente porque o legado funciona.

---

## 10. Registro Automático

Registro Automático faz parte da mesma Foundation de conectividade, mas não deve ser tratado como uma variação do P2P.

Será um transporte distinto.

Direção:

- dispositivo estabelece/inicia o vínculo com o SENTRIUM ou serviço de registro;
- protocolo de registro separado de login NetSDK;
- RealPlay continua independente;
- APIs/callbacks devem ser confirmados nos headers/SDK oficiais antes de qualquer implementação.

Nenhuma API específica será inventada ou presumida.

---

## 11. Próxima auditoria técnica

Antes de implementar Cloud/P2P:

1. localizar os binários usados pelo legado;
2. registrar hashes e arquitetura PE;
3. enumerar exports reais de `SIMNext.P2PSDK.dll`, `Libt2uWrapper.dll` e `libt2u.dll`;
4. identificar quais exports o legado realmente consome;
5. comparar isso com o código legado/auditoria existente;
6. descobrir os parâmetros mínimos de inicialização;
7. identificar como UID/serial/dispositivo é resolvido;
8. comprovar como é reportado P2P direto versus Relay;
9. testar criação e destruição do túnel isoladamente;
10. somente depois acoplar login NetSDK;
11. validar reconexão;
12. documentar blockers de redistribuição e clean install.

O objetivo é repetir a estratégia usada na F003/F004: **subset mínimo comprovado**, sem portar wrappers gigantes.

---

## 12. Critérios preliminares de sucesso do Cloud

A futura implementação deverá demonstrar, no mínimo:

- conexão por identificador Cloud real;
- diferenciação P2P direto/Relay;
- login NetSDK através do túnel;
- obtenção da quantidade correta de canais;
- disconnect/reconnect;
- nenhuma credencial em logs;
- teardown sem processo/handle residual;
- perda de rede e recuperação controlada;
- mudança de transporte sem duplicar dispositivo;
- RealPlay Principal/Extra funcionando sobre a sessão Cloud;
- funcionamento sem SIM Next instalado, se a licença/SDK permitir distribuição independente.

---

## 13. O que ainda NÃO está comprovado

Até este documento:

- Cloud/P2P ainda não foi implementado no C++ do SENTRIUM;
- nenhum novo túnel P2P foi aberto pela Foundation-005;
- Relay ainda não foi medido no SENTRIUM;
- não foi comprovada redistribuição independente dos binários;
- não foi validado clean install;
- não foi auditado Registro Automático no SDK atual;
- não existem benchmarks 16/32 canais via Cloud no SENTRIUM.

Esses itens permanecem trabalho futuro e não devem aparecer como PASS.

---

## 14. Estado consolidado

### Comprovado

- LAN/IP direto em C++;
- sessão NetSDK real;
- canais reais;
- Principal/Extra;
- pipeline de mídia independente;
- existência de caminho Cloud/P2P funcional no legado.

### Decidido

- F005 priorizará Device Connectivity;
- LAN, Cloud/P2P e Registro Automático serão transportes distintos;
- mesma identidade lógica de dispositivo;
- P2P direto azul e Relay amarelo;
- vida útil do túnel separada de RealPlay;
- implementação mínima, baseada em exports reais.

### Pendente

- auditoria dos binários Cloud;
- contrato mínimo C++;
- implementação P2P;
- Relay;
- clean install;
- licença/redistribuição;
- Registro Automático;
- benchmarks com NVRs completos.

---

## 15. Princípio de implementação

> **Arquitetura suficiente para o problema real.**

Cloud/P2P deve resolver conectividade, não criar uma segunda arquitetura do VMS.

O objetivo da Foundation-005 será tornar diferentes caminhos de acesso transparentes para o restante do SENTRIUM, preservando comportamento previsível, baixo acoplamento e evidência real antes de qualquer decisão de produção.
