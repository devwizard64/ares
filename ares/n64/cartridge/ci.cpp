auto Cartridge::CI::readCard(Memory::Writable16 buf, u32 address, u32 len, u8 swap) -> void {
  status = 0x1000;
  if (auto fp = fopen("card.img", "rb")) {
    fseek(fp, 0, SEEK_END);
    if (512*(lba+len) <= ftell(fp)) {
      fseek(fp, 512*lba, SEEK_SET);
      for (u32 i = 0; i < 512*len; i += 2) {
        u8 a = fgetc(fp);
        u8 b = fgetc(fp);
        buf.write<Half>(address+i, swap ? (b << 8 | a) : (a << 8 | b));
      }
      status = 0;
    }
    fclose(fp);
  }
}

auto Cartridge::CI::writeCard(Memory::Writable16 buf, u32 address, u32 len) -> void {
  status = 0x1000;
  if (auto fp = fopen("card.img", "r+b")) {
    fseek(fp, 0, SEEK_END);
    if (512*(lba+len) <= ftell(fp)) {
      fseek(fp, 512*lba, SEEK_SET);
      for (u32 i = 0; i < 512*len; i += 2) {
        u16 data = buf.read<Half>(address+i);
        fputc(data >> 8, fp);
        fputc(data >> 0, fp);
      }
      status = 0;
    }
    fclose(fp);
  }
}

auto Cartridge::CI::readWord(u32 address) -> u32 {
  u32 data = 0;
  switch (address & 0xFFFC)
  {
  case 0x0200:
    data = status;
    break;
  case 0x0218:
    data = length;
    break;
  case 0x0220:
    data = 0;
    break;
  case 0x02E8:
    data = 64_MiB;
    break;
  case 0x02EC:
    data = 0x55444556;
    break;
  case 0x02F0:
    data = 0x4100;
    break;
  case 0x02FC:
    data = 0;
    break;
  }
  return data;
}

auto Cartridge::CI::writeWord(u32 address, u32 data) -> void {
  switch (address & 0xFFFC) {
  case 0x0208:
    switch (data)
    {
    case 0x01:
      readCard(buffer, 0, 1, 0);
      break;
    case 0x03:
      readCard(cartridge.rom, buffer.read<Word>(4) << 1 & 0x3FFFFFF, length, byteswap);
      break;
    case 0x10:
      writeCard(buffer, 0, 1);
      break;
    case 0x13:
      writeCard(cartridge.rom, buffer.read<Word>(4) << 1 & 0x3FFFFFF, length);
      break;
    case 0xE0:
      byteswap = 0;
      break;
    case 0xE1:
      byteswap = 1;
      break;
    case 0xF0:
      cartrom = 0;
      break;
    case 0xF1:
      cartrom = 1;
      break;
    case 0xFF:
      status = 0;
      break;
    }
    break;
  case 0x0210:
    lba = data;
    break;
  case 0x0218:
    length = data;
    break;
  }
}
