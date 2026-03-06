#ifndef CRYPTO_H
#define CRYPTO_H

void initCrypto();
void updatePrices();
void calculateChanges();
float getOldPrice(int asset_index);
float getBinancePrice(const char* symbol);

#endif // CRYPTO_H
