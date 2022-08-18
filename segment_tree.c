const int NUM_LEAF = 15; // log of number of leaves
const int FIRST_LEAF_ID = 1<<(NUM_LEAF);
const int BIT_TREE_SIZE = 1<<(NUM_LEAF+1) / 8;
const int NUM_BIT_PER_BITMAP = 64;
const int LEAF_BITMAP_SIZE = NUM_BIT_PER_BITMAP/8;
char bit_tree[BIT_TREE_SIZE];
char leaf_bitmap[LEAF_BITMAP_SIZE];

/**
 * @brief tra lai gia tri cua bit thu bit_num cua buff 
 *
 * @param buff 
 * @param bit_num 
 * @return char 
 */
char get_bit_num(char *buff, int bit_num) {
    bit_num --;
    int x = bit_num / 8;
    int y = bit_num % 8;
    buff += 8 * x;
    return (buff[0] >> y) & 1;
}
/**
 * @brief viet bit thu bit_num gia tri val
 * 
 * @param buff 
 * @param bit_num 
 * @param val 
 */
void write_bit(char *buff, int bit_num, char val) {
    bit_num --;
    int x = bit_num / 8;
    int y = bit_num % 8;
    char mask = ~(1<<y); // all bit = 1 except bit y
    buff[0] = (buff[0] & mask) | ((1 << y) * val);
}

/**
 * @brief 
 * 
 * @param tree 
 * @param node_id 
 * @return int 
 */
int find_first_free_leaf(char *tree) {
    return backend_find_first_free_feaf(tree, 1);
}
int backend_find_first_free_leaf(char *tree, int node_id) {
    if(node_id >= FIRST_LEAF_ID) {
        int tmp = get_bit_num(tree, node_id);
        if(tmp == 1) return 0;
        else return tmp;
    }
    
    if(get_bit_num(tree, node_id) == 1) return 0;
    
    int left = get_bit_num(tree, 2 * node_id);
    if(left == 0) {
        int right = get_bit_num(tree, 2 * node_id + 1);
        if (right == 1) return 0;
        else return  2 * node_id + 1;
    } else return 2 * node_id;

}
/**
 * @brief 
 * 
 * @param buff 
 * @return int 
 */
int find_first_free_bit_in_leaf(char* buff) {
    // num_bit start at 1
    for (int i=1; i<=NUM_BIT_PER_BITMAP; i++) {
        if(get_bit_num(buff, i)==0)
            return i;
    }
    return 0;
}
/**
 * @brief doc bitmap cua leaf_id vao leaf_bitmap va tra ve con tro toi leaf_bitmap
 * 
 * @param leaf_id 
 * @return char* 
 */
char *get_leaf_bitmap(int leaf_id) {
    // TODO: doc bitmat cua block thu leaf_id - first_leaf_id + 1
    //coi nhu block bat dau tu 1
}
/**
 * @brief 
 * 
 * @param buff 
 * @return int 
 */
int find_first_free_bit(char *tree) {
    int free_leaf_id = find_first_free_leaf(tree);
    if(free_leaf_id == 0) return 0;
    // TODO: doc bitmap cua leaf thu free_leaf_id
    char *leaf_bitmap = get_leaf_bitmap(free_leaf_id);
    int free_bit = find_first_free_bit_in_leaf(leaf_bitmap);
    return NUM_BIT_PER_BITMAP * (free_leaf_id - 1) + free_bit;
}
void update_leaf(char *tree, int leaf_id) {
    char *leaf_bitmap = get_leaf_bitmap(leaf_id);
    int bit_num = find_first_free_bit_in_leaf(leaf_bitmap);
    if(bit_num == 1) return;
    write_bit(tree, leaf_id, 1);
    int current_node = leaf_id;
    char cur_bit = 1;
    while(current_node > 1) {
        int parent = current_node / 2;
        int other_child = 2*parent + (current_node % 2) ^ 1;
        char other_bit = get_bit_num(tree, other_child);
        cur_bit = cur_bit & other_bit;
        if(cur_bit == 0) break;
        write_bit(tree, parent, 1);
    }
}
void opcupy_bitnum(char* tree, int bit_id) {
    int leaf_num = (bit_id + NUM_BIT_PER_BITMAP - 1) / NUM_BIT_PER_BITMAP;
    int leaf_id = FIRST_LEAF_ID + leaf_num - 1;
    char* buff = get_leaf_bitmap(leaf_id);
    write_bit(buff, bit_id % NUM_BIT_PER_BITMAP, 1)
    update_leaf(tree, leaf_id);
}
void free_bitnum(char* tree, int bit_id) {
    int leaf_num = (bit_id + NUM_BIT_PER_BITMAP - 1) / NUM_BIT_PER_BITMAP;
    int leaf_id = FIRST_LEAF_ID + leaf_num - 1;
    char* buff = get_leaf_bitmap(leaf_id);
    write_bit(buff, bit_id % NUM_BIT_PER_BITMAP, 0)
    update_leaf(tree, leaf_id);
}
int main() {

}
