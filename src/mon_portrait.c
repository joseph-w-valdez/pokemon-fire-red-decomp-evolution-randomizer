#include "global.h"
#include "gflib.h"
#include "mon_portrait.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "sprite.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"

static EWRAM_DATA u8 sMonPortraitSpriteId = MON_PORTRAIT_NONE;
static EWRAM_DATA u8 sMonPortraitMode = MON_PORTRAIT_MODE_ICON;

void MonPortrait_SetDefaults(struct MonPortraitConfig *config)
{
    if (config == NULL)
        return;

    config->usePartySlot = TRUE;
    config->partySlot = 0;
    config->species = SPECIES_NONE;
    config->personality = 0;
    config->otId = 0;
    config->windowId = MON_PORTRAIT_NO_WINDOW;
    config->align = MON_PORTRAIT_ALIGN_ABS;
    config->x = 0;
    config->y = 0;
    config->nudgeX = 0;
    config->nudgeY = 0;
    config->pad = 8;
    config->oamPriority = 0;
    config->subpriority = 0;
    config->animated = TRUE;
    config->mode = MON_PORTRAIT_MODE_ICON;
    config->paletteSlot = 12;
}

static void MonPortrait_ResolvePosition(const struct MonPortraitConfig *config, s16 *outX, s16 *outY)
{
    s16 left;
    s16 top;
    s16 width;
    s16 height;
    s16 half;

    if (config->align == MON_PORTRAIT_ALIGN_ABS || config->windowId == MON_PORTRAIT_NO_WINDOW)
    {
        *outX = config->x + config->nudgeX;
        *outY = config->y + config->nudgeY;
        return;
    }

    left = GetWindowAttribute(config->windowId, WINDOW_TILEMAP_LEFT) * 8;
    top = GetWindowAttribute(config->windowId, WINDOW_TILEMAP_TOP) * 8;
    width = GetWindowAttribute(config->windowId, WINDOW_WIDTH) * 8;
    height = GetWindowAttribute(config->windowId, WINDOW_HEIGHT) * 8;
    half = (config->mode == MON_PORTRAIT_MODE_FRONT_PIC) ? 32 : (MON_PORTRAIT_ICON_SIZE / 2);

    switch (config->align)
    {
    case MON_PORTRAIT_ALIGN_LEFT_MIDDLE:
        *outX = left + half + config->pad;
        *outY = top + height / 2;
        break;
    case MON_PORTRAIT_ALIGN_CENTER:
        *outX = left + width / 2;
        *outY = top + height / 2;
        break;
    case MON_PORTRAIT_ALIGN_RIGHT_MIDDLE:
    default:
        *outX = left + width - half - config->pad;
        *outY = top + height / 2;
        break;
    }

    *outX += config->nudgeX;
    *outY += config->nudgeY;
}

u8 MonPortrait_Show(const struct MonPortraitConfig *config)
{
    struct Pokemon *mon;
    u16 species;
    u32 personality;
    u32 otId;
    u16 spriteId;
    s16 x;
    s16 y;
    SpriteCallback callback;

    if (config == NULL)
        return MON_PORTRAIT_NONE;

    MonPortrait_Destroy();
    MonPortrait_ResolvePosition(config, &x, &y);

    if (config->usePartySlot)
    {
        if (config->partySlot >= PARTY_SIZE)
            return MON_PORTRAIT_NONE;
        mon = &gPlayerParty[config->partySlot];
        species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
        if (species == SPECIES_NONE)
            return MON_PORTRAIT_NONE;
        personality = GetMonData(mon, MON_DATA_PERSONALITY);
        otId = GetMonData(mon, MON_DATA_OT_ID);
    }
    else
    {
        species = config->species;
        if (species == SPECIES_NONE)
            return MON_PORTRAIT_NONE;
        personality = config->personality;
        otId = config->otId;
    }

    sMonPortraitMode = config->mode;

    if (config->mode == MON_PORTRAIT_MODE_FRONT_PIC)
    {
        spriteId = CreateMonPicSprite_HandleDeoxys(species, otId, personality, TRUE,
                                                   x, y, config->paletteSlot, TAG_NONE);
        if (spriteId == 0xFFFF)
            return MON_PORTRAIT_NONE;
        FreeSpriteOamMatrix(&gSprites[spriteId]);
        gSprites[spriteId].hFlip = !IsMonSpriteNotFlipped(species);
    }
    else
    {
        LoadMonIconPalettes();
        callback = config->animated ? SpriteCB_MonIcon : SpriteCallbackDummy;
        spriteId = CreateMonIcon(species, callback, x, y, config->subpriority, personality, TRUE);
        if (spriteId == MAX_SPRITES)
            return MON_PORTRAIT_NONE;
    }

    gSprites[spriteId].oam.priority = config->oamPriority;
    sMonPortraitSpriteId = spriteId;
    return sMonPortraitSpriteId;
}

u8 MonPortrait_ShowPartySlot(u8 partySlot, s16 x, s16 y)
{
    struct MonPortraitConfig config;

    MonPortrait_SetDefaults(&config);
    config.partySlot = partySlot;
    config.align = MON_PORTRAIT_ALIGN_ABS;
    config.x = x;
    config.y = y;
    return MonPortrait_Show(&config);
}

u8 MonPortrait_ShowPartySlotInWindow(u8 partySlot, u8 windowId, s16 nudgeX, s16 nudgeY)
{
    struct MonPortraitConfig config;

    MonPortrait_SetDefaults(&config);
    config.partySlot = partySlot;
    config.windowId = windowId;
    config.align = MON_PORTRAIT_ALIGN_RIGHT_MIDDLE;
    config.nudgeX = nudgeX;
    config.nudgeY = nudgeY;
    return MonPortrait_Show(&config);
}

void MonPortrait_Hide(bool8 hide)
{
    if (sMonPortraitSpriteId != MON_PORTRAIT_NONE)
        gSprites[sMonPortraitSpriteId].invisible = hide;
}

void MonPortrait_Destroy(void)
{
    if (sMonPortraitSpriteId == MON_PORTRAIT_NONE)
        return;

    if (sMonPortraitMode == MON_PORTRAIT_MODE_FRONT_PIC)
        FreeAndDestroyMonPicSprite(sMonPortraitSpriteId);
    else
    {
        DestroyMonIcon(&gSprites[sMonPortraitSpriteId]);
        FreeMonIconPalettes();
    }
    sMonPortraitSpriteId = MON_PORTRAIT_NONE;
}

u8 MonPortrait_GetSpriteId(void)
{
    return sMonPortraitSpriteId;
}
