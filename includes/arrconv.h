/*
 * Copyright (C) 2018 Alexzander Purwoko Widiantoro
 */
#include <stdio.h>
#include <stdlib.h>
/*
 * size_t __getlenis(str, separator);
 * Gets the length of array that specified in the list {@see @param str}
 * @param str The list of array, separated with a separator {@link separator}
 * @param separator The Separator
 * @return The length of list/array stored in string str
 */
size_t __getlenis(const char *str, char separator){
	if(!str)return 0;
	int x = 0;
	size_t count = 0;
	for(; str[x] != '\0'; x++){
		if(str[x] == separator)
			count++;
	}
	return (x || count) ? count + 1: 0;
}
/*
 * int *__strtointp(str, separator);
 * Gets an array of int that specified in string str
 * @param str the list of int array separated with (@see @param separator)
 * @param separator the separator
 * @return the pointer of an int array, must be freed after an operation
 */
int *__strtointp(const char *str, char separator){
	size_t len = __getlenis(str, separator);
	if(!len)return NULL;
	int *__result = (int *) malloc(len * sizeof(int));
	int __temp = 0;
	int __x = 0, __y = 0;
	while(1){
		if(str[__x] == separator){
			__result[__y++] = __temp;
			__x++;
			__temp = 0;
			continue;
		}
		else if(str[__x] == '\0'){
			__result[__y++] = __temp;
			break;
		}
		__temp+=(str[__x] - '0');
		__temp = (str[__x+1] == separator || str[__x+1] == '\0')? __temp * 1 : __temp * 10;
		__x++;
	}
	return __result;
}
