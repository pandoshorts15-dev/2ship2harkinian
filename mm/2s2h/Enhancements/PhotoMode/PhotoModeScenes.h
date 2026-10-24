#pragma once
// Lista de escenarios para el teletransporte del Modo Foto.
// Cada entrada usa la entrada 0 de la escena (la misma que usa el selector de mapas de 2Ship).
// "alias" es el nombre en ingles, para que el buscador encuentre ambos.

struct PhotoModeScene {
    const char* region;
    const char* label;
    const char* alias;
    s32 entrance;
};

#define PM_SCENE(region, label, alias, scene) { region, label, alias, ENTRANCE(scene, 0) }

static const PhotoModeScene sPhotoModeScenes[] = {
    // Ciudad Reloj
    PM_SCENE("Ciudad Reloj", "Ciudad Reloj Sur", "South Clock Town", SOUTH_CLOCK_TOWN),
    PM_SCENE("Ciudad Reloj", "Ciudad Reloj Este", "East Clock Town", EAST_CLOCK_TOWN),
    PM_SCENE("Ciudad Reloj", "Ciudad Reloj Oeste", "West Clock Town", WEST_CLOCK_TOWN),
    PM_SCENE("Ciudad Reloj", "Ciudad Reloj Norte", "North Clock Town", NORTH_CLOCK_TOWN),
    PM_SCENE("Ciudad Reloj", "Lavadero", "Laundry Pool", LAUNDRY_POOL),
    PM_SCENE("Ciudad Reloj", "Interior de la Torre del Reloj", "Clock Tower Interior", CLOCK_TOWER_INTERIOR),
    PM_SCENE("Ciudad Reloj", "Azotea de la Torre del Reloj", "Clock Tower Rooftop", CLOCK_TOWER_ROOFTOP),
    PM_SCENE("Ciudad Reloj", "Residencia del alcalde", "Mayor's Residence", MAYORS_RESIDENCE),
    PM_SCENE("Ciudad Reloj", "Observatorio astral", "Astral Observatory", ASTRAL_OBSERVATORY),
    PM_SCENE("Ciudad Reloj", "Escuela de esgrima", "Swordsman's School", SWORDMANS_SCHOOL),
    PM_SCENE("Ciudad Reloj", "Oficina de correos", "Post Office", POST_OFFICE),
    PM_SCENE("Ciudad Reloj", "Posada (Stock Pot Inn)", "Stock Pot Inn", STOCK_POT_INN),
    PM_SCENE("Ciudad Reloj", "Bar de leche (Milk Bar)", "Milk Bar", MILK_BAR),
    PM_SCENE("Ciudad Reloj", "Tienda de curiosidades", "Curiosity Shop", CURIOSITY_SHOP),
    PM_SCENE("Ciudad Reloj", "Bazar (Trading Post)", "Trading Post", TRADING_POST),
    PM_SCENE("Ciudad Reloj", "Tienda de bombas", "Bomb Shop", BOMB_SHOP),
    PM_SCENE("Ciudad Reloj", "Tienda del cofre del tesoro", "Treasure Chest Shop", TREASURE_CHEST_SHOP),
    PM_SCENE("Ciudad Reloj", "Tienda de Honey y Darling", "Honey & Darling's Shop", HONEY_AND_DARLINGS_SHOP),
    PM_SCENE("Ciudad Reloj", "Galeria de tiro (ciudad)", "Town Shooting Gallery", TOWN_SHOOTING_GALLERY),
    PM_SCENE("Ciudad Reloj", "Tienda de loteria", "Lottery Shop", LOTTERY_SHOP),
    PM_SCENE("Ciudad Reloj", "Parque de los Deku", "Deku Scrub Playground", DEKU_SCRUB_PLAYGROUND),
    PM_SCENE("Ciudad Reloj", "Fuente de las hadas", "Fairy's Fountain", FAIRY_FOUNTAIN),

    // Campo de Termina y rancho
    PM_SCENE("Campo y rancho", "Campo de Termina", "Termina Field", TERMINA_FIELD),
    PM_SCENE("Campo y rancho", "Antes de Ciudad Reloj (inicio)", "Before Clock Town", OPENING_DUNGEON),
    PM_SCENE("Campo y rancho", "Bosque perdido (intro)", "Lost Woods", LOST_WOODS),
    PM_SCENE("Campo y rancho", "Camino de la leche (Milk Road)", "Milk Road", MILK_ROAD),
    PM_SCENE("Campo y rancho", "Rancho Romani", "Romani Ranch", ROMANI_RANCH),
    PM_SCENE("Campo y rancho", "Casa y establo del rancho", "Mama's House & Barn", RANCH_HOUSE),
    PM_SCENE("Campo y rancho", "Corral de los cucos", "Cucco Shack", CUCCO_SHACK),
    PM_SCENE("Campo y rancho", "Carrera de perros", "Doggy Racetrack", DOGGY_RACETRACK),
    PM_SCENE("Campo y rancho", "Pista de los Gorman", "Gorman Track", GORMAN_TRACK),

    // Pantano
    PM_SCENE("Pantano", "Camino al Pantano del Sur", "Road to Southern Swamp", ROAD_TO_SOUTHERN_SWAMP),
    PM_SCENE("Pantano", "Galeria de tiro (pantano)", "Swamp Shooting Gallery", SWAMP_SHOOTING_GALLERY),
    PM_SCENE("Pantano", "Pantano del Sur (envenenado)", "Southern Swamp (poison)", SOUTHERN_SWAMP_POISONED),
    PM_SCENE("Pantano", "Pantano del Sur (limpio)", "Southern Swamp (clear)", SOUTHERN_SWAMP_CLEARED),
    PM_SCENE("Pantano", "Centro de informacion turistica", "Tourist Information", TOURIST_INFORMATION),
    PM_SCENE("Pantano", "Tienda de pociones de las brujas", "Magic Hags' Potion Shop", MAGIC_HAGS_POTION_SHOP),
    PM_SCENE("Pantano", "Bosque misterioso", "Woods of Mystery", WOODS_OF_MYSTERY),
    PM_SCENE("Pantano", "Casa Skulltula del pantano", "Swamp Spider House", SWAMP_SPIDER_HOUSE),
    PM_SCENE("Pantano", "Palacio Deku", "Deku Palace", DEKU_PALACE),
    PM_SCENE("Pantano", "Sala del Rey Deku", "Deku King's Chamber", DEKU_KINGS_CHAMBER),
    PM_SCENE("Pantano", "Santuario Deku", "Deku Shrine", DEKU_SHRINE),
    PM_SCENE("Pantano", "Woodfall", "Woodfall", WOODFALL),
    PM_SCENE("Pantano", "Templo de Woodfall", "Woodfall Temple", WOODFALL_TEMPLE),
    PM_SCENE("Pantano", "Guarida de Odolwa", "Odolwa's Lair", ODOLWAS_LAIR),

    // Montana
    PM_SCENE("Montana", "Camino a la Aldea de la Montana", "Path to Mountain Village", PATH_TO_MOUNTAIN_VILLAGE),
    PM_SCENE("Montana", "Aldea de la Montana (invierno)", "Mountain Village (winter)", MOUNTAIN_VILLAGE_WINTER),
    PM_SCENE("Montana", "Aldea de la Montana (primavera)", "Mountain Village (spring)", MOUNTAIN_VILLAGE_SPRING),
    PM_SCENE("Montana", "Herreria", "Mountain Smithy", MOUNTAIN_SMITHY),
    PM_SCENE("Montana", "Camino a la Aldea Goron (invierno)", "Path to Goron Village (winter)",
             PATH_TO_GORON_VILLAGE_WINTER),
    PM_SCENE("Montana", "Camino a la Aldea Goron (primavera)", "Path to Goron Village (spring)",
             PATH_TO_GORON_VILLAGE_SPRING),
    PM_SCENE("Montana", "Aldea Goron (invierno)", "Goron Village (winter)", GORON_VILLAGE_WINTER),
    PM_SCENE("Montana", "Aldea Goron (primavera)", "Goron Village (spring)", GORON_VILLAGE_SPRING),
    PM_SCENE("Montana", "Santuario Goron", "Goron Shrine", GORON_SHRINE),
    PM_SCENE("Montana", "Tienda Goron", "Goron Shop", GORON_SHOP),
    PM_SCENE("Montana", "Cementerio Goron", "Goron Graveyard", GORON_GRAVERYARD),
    PM_SCENE("Montana", "Pista de carreras Goron", "Goron Racetrack", GORON_RACETRACK),
    PM_SCENE("Montana", "Camino a Snowhead", "Path to Snowhead", PATH_TO_SNOWHEAD),
    PM_SCENE("Montana", "Snowhead", "Snowhead", SNOWHEAD),
    PM_SCENE("Montana", "Templo de Snowhead", "Snowhead Temple", SNOWHEAD_TEMPLE),
    PM_SCENE("Montana", "Guarida de Goht", "Goht's Lair", GOHTS_LAIR),

    // Gran Bahia
    PM_SCENE("Gran Bahia", "Costa de la Gran Bahia", "Great Bay Coast", GREAT_BAY_COAST),
    PM_SCENE("Gran Bahia", "Cabo Zora", "Zora Cape", ZORA_CAPE),
    PM_SCENE("Gran Bahia", "Cabana del pescador", "Fisherman's Hut", FISHERMANS_HUT),
    PM_SCENE("Gran Bahia", "Laboratorio marino", "Marine Research Lab", MARINE_RESEARCH_LAB),
    PM_SCENE("Gran Bahia", "Casa Skulltula del oceano", "Oceanside Spider House", OCEANSIDE_SPIDER_HOUSE),
    PM_SCENE("Gran Bahia", "Roca Pinaculo", "Pinnacle Rock", PINNACLE_ROCK),
    PM_SCENE("Gran Bahia", "Rapidos de la cascada", "Waterfall Rapids", WATERFALL_RAPIDS),
    PM_SCENE("Gran Bahia", "Salon Zora", "Zora Hall", ZORA_HALL),
    PM_SCENE("Gran Bahia", "Habitaciones del Salon Zora", "Zora Hall Rooms", ZORA_HALL_ROOMS),
    PM_SCENE("Gran Bahia", "Foso de la Fortaleza Pirata", "Pirates' Fortress Moat", PIRATES_FORTRESS_EXTERIOR),
    PM_SCENE("Gran Bahia", "Fortaleza Pirata", "Pirates' Fortress", PIRATES_FORTRESS),
    PM_SCENE("Gran Bahia", "Interior de la Fortaleza Pirata", "Pirates' Fortress Interior",
             PIRATES_FORTRESS_INTERIOR),
    PM_SCENE("Gran Bahia", "Templo de la Gran Bahia", "Great Bay Temple", GREAT_BAY_TEMPLE),
    PM_SCENE("Gran Bahia", "Guarida de Gyorg", "Gyorg's Lair", GYORGS_LAIR),

    // Ikana
    PM_SCENE("Ikana", "Camino a Ikana", "Road to Ikana", ROAD_TO_IKANA),
    PM_SCENE("Ikana", "Cementerio de Ikana", "Ikana Graveyard", IKANA_GRAVEYARD),
    PM_SCENE("Ikana", "Bajo el cementerio", "Beneath the Graveyard", BENEATH_THE_GRAVERYARD),
    PM_SCENE("Ikana", "Casa de Dampe", "Dampe's House", DAMPES_HOUSE),
    PM_SCENE("Ikana", "Canon de Ikana", "Ikana Canyon", IKANA_CANYON),
    PM_SCENE("Ikana", "Escondite de Sakon", "Sakon's Hideout", SAKONS_HIDEOUT),
    PM_SCENE("Ikana", "Casa de la caja de musica", "Music Box House", MUSIC_BOX_HOUSE),
    PM_SCENE("Ikana", "Casa de los espiritus", "Ghost Hut", GHOST_HUT),
    PM_SCENE("Ikana", "Bajo el pozo", "Beneath the Well", BENEATH_THE_WELL),
    PM_SCENE("Ikana", "Antiguo Castillo de Ikana", "Ancient Castle of Ikana", IKANA_CASTLE),
    PM_SCENE("Ikana", "Sala de Igos du Ikana", "Igos du Ikana's Lair", IGOS_DU_IKANAS_LAIR),
    PM_SCENE("Ikana", "Santuario secreto", "Secret Shrine", SECRET_SHRINE),
    PM_SCENE("Ikana", "Torre de Piedra", "Stone Tower", STONE_TOWER),
    PM_SCENE("Ikana", "Torre de Piedra (invertida)", "Inverted Stone Tower", STONE_TOWER_INVERTED),
    PM_SCENE("Ikana", "Templo de la Torre de Piedra", "Stone Tower Temple", STONE_TOWER_TEMPLE),
    PM_SCENE("Ikana", "Templo de la Torre de Piedra (invertido)", "Inverted Stone Tower Temple",
             STONE_TOWER_TEMPLE_INVERTED),
    PM_SCENE("Ikana", "Guarida de Twinmold", "Twinmold's Lair", TWINMOLDS_LAIR),

    // Final
    PM_SCENE("Final", "Camara de los Gigantes", "Giants' Chamber", GIANTS_CHAMBER),
    PM_SCENE("Final", "La Luna", "The Moon", THE_MOON),
    PM_SCENE("Final", "Luna - prueba Deku", "Moon Deku Trial", MOON_DEKU_TRIAL),
    PM_SCENE("Final", "Luna - prueba Goron", "Moon Goron Trial", MOON_GORON_TRIAL),
    PM_SCENE("Final", "Luna - prueba Zora", "Moon Zora Trial", MOON_ZORA_TRIAL),
    PM_SCENE("Final", "Luna - prueba de Link", "Moon Link Trial", MOON_LINK_TRIAL),
    PM_SCENE("Final", "Guarida de Majora", "Majora's Lair", MAJORAS_LAIR),
};

#undef PM_SCENE

static const char* sPhotoModeRegions[] = { "Ciudad Reloj", "Campo y rancho", "Pantano", "Montana",
                                           "Gran Bahia",   "Ikana",          "Final" };

#define PHOTO_MODE_SCENE_COUNT (sizeof(sPhotoModeScenes) / sizeof(sPhotoModeScenes[0]))
#define PHOTO_MODE_REGION_COUNT (sizeof(sPhotoModeRegions) / sizeof(sPhotoModeRegions[0]))
