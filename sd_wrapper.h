int sd_init();

int sd_read_block(uint32_t block, uint8_t* result);

int sd_write_block(uint32_t block, uint8_t* result);
