# Rendering & Visibility {#sdk_visibility_guide}

[TOC]

## EntityRender

Mutate `m_nRenderMode` and `m_clrRender` on any `CBaseModelEntity`. Used internally by `PlayerController::SetVisible`, but exposed so plugins can hide/recolor any entity (props, dropped weapons, world objects).

```cpp
using namespace CS2Kit::Sdk;

SetEntityRender(prop, RenderMode_t::TransTexture, ColorInvisible);
SetEntityRender(prop, RenderMode_t::Normal, ColorOpaqueWhite);
```

`m_clrRender` is RGBA packed as `(A << 24) | (B << 16) | (G << 8) | R` - `ColorInvisible` (`0x00FFFFFF`) is white at zero alpha.

Render tricks only affect the pawn body; held weapons, wearables and gloves are separate networked entities. For true invisibility use the TransmitFilter instead.

## TransmitFilter

Per-recipient entity transmit filtering via a post-hook on `ISource2GameEntities::CheckTransmit`. Hidden entities are never sent to the client, so the model, weapons, wearables, gloves and shadow all disappear - unlike render alpha. Two independent per-slot toggles:

```cpp
auto& transmit = Engine().Transmit;

transmit.SetPawnHidden(slot, true);        // pawn + weapons + wearables vanish for everyone else
transmit.SetControllerHidden(slot, true);  // removes the player's scoreboard row
```

The hidden player still receives their own entities, and a client actively observing the hidden pawn keeps receiving it (dropping it would break the spectator camera). Sounds (footsteps, gunfire) are networked separately and are not filtered. State is cleared automatically when the player disconnects.

Requires the `CheckTransmitPlayerSlot` gamedata offset (the recipient slot inside the partially-reversed `CCheckTransmitInfo`); if it is missing the service logs a warning at load and becomes inert.
