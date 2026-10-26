# F005 Cloud R2 — PTCP framing + device-owned cryptography

**Data:** 26/10/2026  
**Branch:** `foundation/005-device-connectivity`  
**Situação:** código C++ implementado / compilação Windows **NÃO EXECUTADA NESTE LOTE** / conexão Cloud real **NÃO COMPROVADA**.

## Contexto factual

O log real anterior do operador, referente ao commit `4185a7000940c2c31f8dbdd050198f5070429bf1`, comprovou PRE-FLIGHT PASS, CMake Debug Build PASS `[46/46]`, D-9 PASS e D-10 PASS. Isso se referia ao parser DHP2P e à camada UDP, sem Cloud remoto.

O próximo avanço deve ser sobre **framing PTCP e autenticação de canal do dispositivo**, sem repetir a mesma bateria de verificações da F004 nem os D-9/D-10 já aprovados. O usuário pediu entregas consolidadas com um único comando de validação no fim.

## Implementação native C++20

### PTCP — arquivos

- `libs/sentrium-device/include/sentrium/cloud_ptcp.hpp`
- `libs/sentrium-device/cloud_ptcp.cpp`

Codificação/decodificação de datagramas com header de 24 bytes `PTCP`, contadores `Sent`/`Recv`, PID, LMID, RMID (big-endian). Suporta: SYNC, PAYLOAD (`0x10`), BIND (`0x11`), STATUS (`0x12`), HEARTBEAT (`0x13`) e comandos opacos de handshake `0x17` a `0x1b`. Tratamento de comprimento/padding inconsistente, cabeçalho inválido e status desconhecido; fragmentos de payload de saída limitados a 1280 bytes.

**O que PTCP framing NÃO faz ainda:** não cria `Session` confiável, não gerencia PID exclusivo, LMID ordenado, ACK, retransmissão, reordenação, timeout, hole punching, Direct/Relay ou fluxo de mídia. O formato do protocolo não autentica a sessão por si.

### Criptografia do canal pertencente ao gravador

- `libs/sentrium-device/include/sentrium/cloud_auth.hpp`
- `libs/sentrium-device/cloud_auth.cpp`

API Windows CNG (`bcrypt.lib`) com MD5 para interoperabilidade **apenas do dispositivo**, HMAC-SHA256, PBKDF2-HMAC-SHA256 (20.000 iterações) e AES-256-OFB dos endereços fornecendo IV explícito. O material é derivado do usuário/senha do próprio NVR e do `randsalt`, nonce e timestamp do dispositivo. Nenhuma chave de aplicativo, senha do SIM Next, endpoint bootstrap ou IV secreto é gravado no repositório.

O objeto `CloudDeviceAuth` contém material **sensível** que não deve aparecer em logs nem diagnósticos, tampouco ser persistido; o buffer temporário com senha composta é sobrescrito após o hashing. O AES-OFB não autentica o conteúdo isoladamente; uma assinatura de protocolo validada continua obrigatória antes de confiar no endereço decodificado.

### Integração e verificação

- `CMakeLists.txt` integra `cloud_ptcp.cpp` e `cloud_auth.cpp`, liga `bcrypt`.
- `tests/device/main.cpp` adiciona `D-11` (PTCP serialização/parse, limites e tipos) e `D-12` (derivações criptográficas determinísticas, IV e AES-OFB round-trip, rejeição de entradas vazias).
- `scripts/windows/sentrium-validate-f005-cloud.ps1` atualiza a validação para **D-11/D-12** (sem repetir D-9/D-10) e guarda output MSVC muito extenso em `out/sentrium-debug/f005-cloud-build-latest.log`, mostrando apenas o final ou últimos 65 registros em falha.
- **Nenhum teste D-11/D-12 recebeu PASS no Windows ainda.** Não inferir resultado do commit anterior.

### Fontes públicas utilizadas como **referência de formato**

- <https://github.com/blackjid/dahua-p2p> — `ptcp/types.go`, `dh/crypto.go`, descrição do handshake; licença MIT. A implementação C++ aqui foi escrita de forma independente. Não acrescentar biblioteca Go, ferramenta `dh-fwd` nem arquivos SIM Next ao build.
- <https://github.com/khoanguyen-3fc/dh-p2p> — referência adicional pública não validada no SENTRIUM.

A documentação de um protocolo reversamente estudado não comprova compatibilidade da infraestrutura Intelbras Cloud utilizada pelo operador. Acesso à infraestrutura e redistribuição permanecem sujeitos à autorização adequada.

## O que ainda bloqueia Cloud real

1. Bootstrap do serviço com identidade/credenciais **legítimas e verificáveis**, não copiadas de aplicativos proprietários.
2. Rendezvous e canal autorizado com resposta real do equipamento.
3. Sessão PTCP completa (ordenar LMID/PID, ACK/retransmit, heartbeat, fragmentação/reassemblagem, perfis NAT).
4. Identificar Direct vs Relay via telemetria efetiva, sem inferência.
5. Apresentar uma conexão TCP local dinâmica funcional ao NetSDK; abrir canais reais e manter túnel por dispositivo.
6. Conexão de campo com NVR próprio de 16/32 canais; reconexão e segurança/distribuição.

Os alertas do Defender sobre `dh-fwd_win_x64.exe` continuam em **SECURITY HOLD**. Sem executáveis terceiros, sem restauração da quarentena ou exceções antivírus.

## Gate Windows (única execução após fechar SENTRIUM)

```powershell
cd "C:\Projetos\SENTRIUM VMS\sentrium-vms"
git pull --ff-only origin foundation/005-device-connectivity
.\scripts\windows\sentrium-validate-f005-cloud.ps1
```

O script faz fast-forward, PRE-FLIGHT, configura e compila Debug, e executa só D-11 e D-12. O requisito de processo `sentrium-client.exe` fechado evita LNK1168.

**Governança:** F005 em implementação, PR #4 draft, main intacta, F004 ainda aguardando Human Review; não declarar Cloud funcional.
