#include "wrapper.h"

/* Kod funkcii my_init, my_alloc a my_free nahradte vlastnym. Nepouzivajte ziadne
 * globalne ani staticke premenne; jedina globalna pamat je dostupna pomocou
 * mread/mwrite/msize, ktorych popis najdete vo wrapper.h */

/* Ukazkovy kod zvladne naraz iba jedinu alokaciu. V 0-tom bajte pamate si
 * pamata, ci je pamat od 1 dalej volna alebo obsadena. 
 *
 * V pripade, ze je volna, volanie my_allloc skonci uspesne a vrati zaciatok
 * alokovanej RAM; my_free pri volnej mamati zlyha.
 *
 * Ak uz nejaka alokacia prebehla a v 0-tom bajte je nenulova hodnota. Nie je
 * mozne spravit dalsiu alokaciu, takze my_alloc musi zlyhat. my_free naopak
 * zbehnut moze a uvolni pamat.
 */


/**
 * Inicializacia pamate
 *
 * Zavola sa, v stave, ked sa zacina s prazdnou pamatou, ktora je inicializovana
 * na 0.
 */
void my_init(void) {

	return;
}

int pow2(int b) {
	if (b < 0) {
		return 0;
	}
	int res = 1;
	for (int i = 0; i < b; i++) {
		res *= 2;
	}
	return res;
}

/**
 * Poziadavka na alokaciu 'size' pamate. 
 *
 * Ak sa pamat podari alokovat, navratova hodnota je adresou prveho bajtu
 * alokovaneho priestoru v RAM. Pokial pamat uz nie je mozne alokovat, funkcia
 * vracia FAIL.
 */
int my_alloc(unsigned int size) {

	/* Nemozeme alokovat viac pamate, ako je dostupne */
	if (size >= msize() - 1)
		return FAIL;

		int currentSize = 0;
		int startingBit = 0;
		int startingByte = 0;
		int spaceNeeded = size;

	/* kazdemu bitu z prvych 1/9 msize() bytov je priradeny jeden byte s datami*/
	/*prvych 1/9 bytov representuje ci ich priradene byte su obsadene*/
	/*medzi kazdymi dvoma obsadenymi blokmi je jeden byte volny*/
	for (unsigned int i = 0; i < msize() / 9; i++) {
		uint8_t flags = mread(i);
		if (flags == 0) currentSize += 8;
		else {
			for (int k = 0; k < 8; k++) {
				if (currentSize > spaceNeeded) break;
				currentSize++;
				if (flags % 2 != 0) {
					if (k == 6) {
						startingBit = 0;
						startingByte = i + 1;
					} else if (k == 7) {
						startingBit = 1;
						startingByte = i + 1;
					} else {
						startingBit = k + 2;
						startingByte = i;
					}
					currentSize = 0;
					spaceNeeded = size + 1;
				}
				flags /= 2;
			}
		}
		if (currentSize > spaceNeeded) {
			if (startingBit + size <= 8) {
				int occupancy = (pow2(startingBit + size) - 1) - (pow2(startingBit) - 1);
				mwrite(startingByte, mread(startingByte) | occupancy);
			} else {
				int inFirstByte = 8 - startingBit;
				int toWrite = size - inFirstByte;
				int occupancy = 255 - (pow2(startingBit) - 1);
				mwrite(startingByte, mread(startingByte) | occupancy);
				for (int j = 1; j <= toWrite / 8; j++) {
					mwrite(startingByte + j, 255);
				}
				occupancy = pow2((toWrite % 8)) - 1;
				int currentPosition = startingByte + (toWrite / 8) + 1;
				mwrite(currentPosition, mread(currentPosition) | occupancy);
			}
			int start = (msize() / 9) + startingByte * 8 + startingBit;
			if (start + size > msize()) {
				return FAIL;
			} else {
				return start;
			}
		}
	}
	return FAIL;
}

/**
 * Poziadavka na uvolnenie alokovanej pamate na adrese 'addr'.
 *
 * Ak bola pamat zacinajuca na adrese 'addr' alokovana, my_free ju uvolni a
 * vrati OK. Ak je adresa 'addr' chybna (nezacina na nej ziadna alokovana
 * pamat), my_free vracia FAIL.
 */

int my_free(unsigned int addr) {
	unsigned int newAddr = addr - (msize() / 9);
	int byte = newAddr / 8;
	int bit = newAddr % 8;
	if ((mread(byte) & pow2(bit)) == 0) {
		return FAIL;
	}
	if (bit == 0 && byte != 0) {
		bit = 7;
		byte = byte - 1;
	} else {
		bit--;
	}
	uint8_t flag = mread(byte);
	if (((flag & (pow2(bit))) == 0) || bit == -1) {
		if (bit == 7) {
			bit = 0;
			byte = byte + 1;
		} else {
			bit++;
		}
		int possition = 0;
		for (int i = 0; i < bit; i++) {
			flag/=2;
		}
		for (int i = bit; i < 8; i++) {
			if (flag % 2 == 0) {
				mwrite(byte, mread(byte) & (255 - (pow2(i + 1) - 1) + pow2(bit) - 1));
				return OK;
			}
			flag /= 2;
		}
		mwrite(byte, mread(byte) & (pow2(bit) - 1));
		int i = 1;
		flag = mread(byte + i);
		while (flag == 255) {
		
			mwrite(byte + i, 0);
			i++;
			if (byte + i >= (msize() / 9)) return OK;
			flag = mread(byte + i);
		}
		for (int j = 0; j < 8; j++) {
			if (flag % 2 == 0) {
				mwrite(byte + i, mread(byte + i) & (255 - (pow2(j) - 1)));
				return OK;
			}
			flag /= 2;
		}
		return FAIL;
	} else {
		return FAIL;
	}
}
