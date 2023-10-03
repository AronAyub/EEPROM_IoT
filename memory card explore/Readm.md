- memory acard comunicate thorugh SPI.
- SPI & SD libaries required.
- SD card does not support larger file name it uses 8.3 filename or SFN naming structure.
- file.write() only writes one byte of data or one array of bytes into the file.
#### Reading and writting structure.

- [Most good libraries have a block read as well, to match the block write. In this case you will need to use a loop to read all the data in one byte at a time. Set a uint8_t pointer to the address of the data/struct and then for each byte you read, assign it to the 'pointed' location and increment the pointer, repeat][def]


[def]: https://forum.arduino.cc/t/reading-writing-struct-to-sd-card/187722/8