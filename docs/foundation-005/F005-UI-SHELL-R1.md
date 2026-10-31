# F005 — Operational UI shell / legacy navigation parity (26/10/2026)

**Status:** IMPLEMENTED IN QT SOURCE / WINDOWS RUNTIME NOT YET VERIFIED  
**Owner-approved reference:** screenshots of XC Video IA Foundation 007 r25, provided in the conversation. The legacy application serves as an interaction/ergonomic reference; do not reuse XC name or copy its internals.

## Existing Qt base preserved

- `DeviceController`, `DeviceWorker`, NetSDK LAN, NetSDK auto-registration by operator-defined ID.
- Existing device tree and real channel list, legacy visualization placeholder, event sidebar and standalone listener controls.
- Separate transport classification LAN / Cloud / AutoRegistration. Cloud backend still BLOCKED pending standalone authenticated bootstrap contract.

## New operational shell

- **Home** is the always-open start tab with module cards and true device counts/status drawn from the controller.
- **Browser-style top tabs** for Home, numbered Visualização workspaces, Dispositivos, Eventos, Regras IA, Mídia, LPR, Configurações. Tabs can be switched/closed without destroying the underlying controller.
- Multiple numbered Visualização tabs can be opened, but **all currently show the same media placeholder**; independent video sessions do NOT exist yet.
- **Dispositivos** module now has a table built from `deviceController.devices`, a filter by name, selection, actual state/channel count, and functional Novo, Conectar, Desconectar, Excluir controls. No synthetic recorder rows.
- **Regras IA**, **LPR**, **Eventos**, **Mídia** have top-level modules and internal tab navigation structures but are explicitly `em desenvolvimento`; no fake IA inference, OCR, recording or event history.
- **Configurações** now opens as a normal workspace tab, with Sistema, Armazenamento, Usuários e permissões, Notificações, Integrações inner tabs; real AutoRegistration listener controls remain in a small settings dialog opened from within this module as an interim implementation.
- **Visualização** retains the earlier left device tree, Visualizar/Reproduzir placeholder, and Eventos Ao Vivo side panel. This is not yet a video mosaic implementation.

## UX invariants

1. Home is permanent and cannot be closed.
2. Clicking `+ Nova visualização` creates another numbered tab, without pretending it creates a video pipeline.
3. Tab title follows SENTRIUM product identity; XC Video IA is a reference, not a runtime dependency.
4. Don't claim Cloud/P2P is available from the UI. Direct/Relay labels require transport evidence.
5. Device state/counts are derived from the real model; no invented connected devices.

## Verification outstanding

The code on branch `foundation/005-device-connectivity` must be **compiled, QML-loaded and visually inspected on Windows**. Static source matching/bracket review is not a replacement for runtime validation. Inspect Home, top tab creation/close, Dispositivos table, inner tabs, and previous LAN/AutoReg flows. Correct build/runtime errors before treating UI as PASS.

**Scope:** F005 in progress; F004 not retroactively approved; no merge to `main`; Human Review pending.

## Ajuste estético adiado por decisão do operador — 26/10/2026

O usuário compartilhou referência visual de uma Home em blocos **ícone quadrado pequeno com cantos arredondados, rótulo centralizado embaixo**, organizado em seções (operações principais e opções avançadas). **Aplicar mais tarde**, preservando Home + abas tipo navegador já implementadas. **Prioridade atual: Cloud/P2P autônomo e Registro Automático real.** Não iniciar nova refatoração estética antes de destravar conectividade funcional.
