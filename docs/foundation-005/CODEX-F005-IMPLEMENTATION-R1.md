# CODEX — SENTRIUM-CPP-Foundation-005 — IMPLEMENTATION R1

**Status:** AUTHORIZED FOR EXECUTION (F005-AUTH-001).  
**Branch:** `foundation/005-device-connectivity`.  
**Principal resultado:** cliente Qt que cadastra, conecta e mostra canais de dispositivos por LAN, Cloud/P2P e Registro Automático, com conectividade **real**, nunca simulações.

## 0. Início

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git status --short
git fetch origin
git switch foundation/005-device-connectivity
git pull --ff-only origin foundation/005-device-connectivity
git rev-parse HEAD
```

Se houver mudanças locais, divergência, ausência da branch, erro de fetch ou preflight, **parar e relatar**, sem reset/clean/stash forçado. A F004 segue intacta; `main` nunca deve ser usada como destino de escrita.

Ler:
- `docs/foundation-005/SENTRIUM-CPP-Foundation-005-SCOPE.md`;
- `docs/foundation-005/F005-DECISION-GATE.md`;
- `docs/foundation-005/SENTRIUM-CPP-Foundation-005-DIRECTION.md`;
- `docs/HANDOFF-SENTRIUM-2026-10-25.md`;
- `apps/sentrium-client/{Main.qml,DeviceController.*,DeviceWorker.*}`;
- `libs/sentrium-device/` e `libs/sentrium-intelbras/`.

## 1. PRE-FLIGHT READ-ONLY curto

Executar `scripts/windows/sentrium-preflight.ps1 -VerifyOnly` e comprovar os paths externos fixados (Qt, NetSDK e PlaySDK). Inspecionar nos locais legados existentes os binários, código-fonte e cabeçalhos de `SIMNext.P2PSDK.dll`, `Libt2uWrapper.dll`, `libt2u.dll` e os exports/contratos relacionados. Não inferir ABI pela nomenclatura. Inspecionar headers NetSDK para as APIs de Auto Registration/Listen/Connect exigidas realmente. Não baixar dependências, não alterar registro/firewall, não introduzir credenciais no código.

Produzir em poucas linhas o inventário técnico necessário à implementação (`VERIFIED / MISSING / INCONCLUSIVE`). **Não produzir novo estudo longo**. Falta de Cloud SDK ou lacuna de Auto Registration bloqueia somente a respectiva funcionalidade, não o avanço no cliente.

## 2. Entregar F005.1 — concluir UI funcional antes de novo benchmark

**Checkpoint já no GitHub:** o commit `e72014ce53001b34018e332e23159e1c032b9984` adicionou a escolha visual das três modalidades no diálogo existente. A ação de conectar permanece ativa **somente para LAN**; Cloud e Registro Automático exibem indisponibilidade real e não enviam ações falsas. **Este patch ainda não foi compilado/testado no Windows**. Não recriar esse seletor; conferir o código e evoluir a partir dele.

Concluir o formulário real por modalidade ao implementar sua respectiva API/SDK, com campos e validação específicos. Reutilizar a mesma árvore de dispositivos/canais e controle de estado. Preservar LAN existente, incluindo disconnect/remove. Enquanto modalidade sem integração estiver indisponível, manter ação desabilitada com explicação.

Usar apenas tipos e rotas necessários em `DeviceController`, `DeviceWorker` e `sentrium-device`. Não construir framework de plugins. Rodar **um build/smoke focal após código funcional**, não repetir benchmarks antigos. Criar commits de código de cada slice real.

## 3. Entregar F005.2 — Cloud/P2P real

Quando os arquivos necessários estiverem comprovadamente disponíveis:
- implementar pequeno adaptador de túnel com vida útil explícita; autenticar o dispositivo remoto sem gravar segredos;
- usar APIs reais do SDK legado/NetSDK verificadas; comprovar login e canais via equipamento real de 16/32 canais disponível;
- registrar modo `Direct`, `Relay` ou `Unknown` *somente* quando o SDK permitir saber; usar azul/amarelo respectivamente, desconhecido neutro;
- separar conexão/túnel, sessão de dispositivo e RealPlay; refazer conexão e liberar recursos corretamente;
- evitar copiar toda a lógica C# antiga; não acrescentar fallback silencioso.

Se binário/header/ABI indispensável não estiver disponível, declarar `F005.2 BLOCKED — SDK MISSING/INCONCLUSIVE` e preservar o código funcional de F005.1; não criar “cloud funcional” falso.

## 4. Entregar F005.3 — Registro Automático real

Somente após comprovar contrato de API/callback e semântica NetSDK:
- implementar listener sob configuração explícita, **desativado por padrão**;
- aceitar associações de dispositivos iniciadas pelos equipamentos, autenticá-los, fornecer DeviceId lógico e estado correto;
- disponibilizar árvore dos canais e shutdown/restart corretos;
- não abrir portas no firewall/NAT, não configurar cloud externo ou serviço público automaticamente;
- registrar erros sem host, senha, serial completo ou dados de cliente.

Sem API comprovada, `F005.3 BLOCKED`, sem mock disfarçado de PASS.

## 5. Validação mínima útil e entrega

- F005.1: abrir o cliente, preservar LAN e demonstrar novo seletor.
- F005.2: **um** NVR remoto real via Cloud/P2P com canais enumerados, se disponível.
- F005.3: **um** registro automático real, se disponível.
- Testes unitários mínimos do código modificado, compilação Debug/Release, ausência de processos residuais e logs saneados.
- Não repetir benchmarks F004 em 2 câmeras nem artificialmente multiplicar canais para alegar escala.

Fazer commits pequenos de implementação. Relatar `PASS / BLOCKED / NOT RUN` por modalidade, indicar SHA, comandos locais para abrir a UI e pendências exatas. Nunca declarar `HUMAN REVIEW APPROVED` ou fazer merge em `main`. F004 mantém status pendente, ADR-005 `Proposed` e F005 só poderá chegar a `READY FOR HUMAN REVIEW` com evidência suficiente.
