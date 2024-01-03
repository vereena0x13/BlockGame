#include "item_simple.cpp"
#include "item_block.cpp"


Item* ITEMS[1024];
itemid N_ITEMS = 0;


#define X(name) Item* item_##name;
ITEMS(X)
#undef X


void register_items() {
    itemid& id = N_ITEMS;

    #define add_item(vn, item) ITEMS[id] = item_##vn = item; id++;

    add_item(bedrock, xnew(Item_Block, id, block_bedrock));
    add_item(stone, xnew(Item_Block, id, block_stone));
    add_item(dirt, xnew(Item_Block, id, block_dirt));
    add_item(grass, xnew(Item_Block, id, block_grass));
    add_item(sand, xnew(Item_Block, id, block_sand));
    add_item(glass, xnew(Item_Block, id, block_glass));
    add_item(iron_ore, xnew(Item_Block, id, block_iron_ore));
    add_item(iron_block, xnew(Item_Block, id, block_iron));
    add_item(iron_ingot, xnew(Item_Simple, id, "iron_ingot"));
    add_item(gold_ore, xnew(Item_Block, id, block_gold_ore));
    add_item(gold_block, xnew(Item_Block, id, block_gold));
    add_item(gold_ingot, xnew(Item_Simple, id, "gold_ingot"));
    add_item(furnace, xnew(Item_Block, id, block_furnace));

    #undef add_item

    log(DEBUG, "Items initialized");
}