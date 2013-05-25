#include "load_map.h"
#include "png.h"
#include "string.h"
#include <stdio.h>

Bitmask* load_from_file(char *filename)
{
    // Stole almost everything from cspotcode GG2DLL: https://github.com/cspotcode/Garrison-Builder/blob/master/GG2DLL/GG2DLL/GG2DLL.cpp
    png_structp image;
    png_infop info;

    image = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info = png_create_info_struct(image);

    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf("Error: Could not find a map with that name!");
    }

    // read the image into RAM
    png_init_io(image, file);

    png_read_info(image, info);

    // We're mostly interested in the text
	png_textp text_ptr;
    int num_text;

	png_get_text(image, info, &text_ptr, &num_text);

    // Find the gg2 text
    int gg2_text_index = -1;
    for(int i = 0; i < num_text; i++)
    {
        if(strcmp(GG2_TEXT_CHUNK_KEYWORD, text_ptr[i].key) == 0)
        {
            gg2_text_index = i;
            break;
        }
    }

    if(gg2_text_index == -1)
    {
        printf("Error: No wallmask data found in map!");
    }

    // Copy the data string to memory we can actually use
    char *text = (char*) calloc(strlen(text_ptr[gg2_text_index].text), 1);
    memcpy(text, text_ptr[gg2_text_index].text, sizeof(char) * strlen(text_ptr[gg2_text_index].text));

    // Setup the object that'll hold the data
    Bitmask *map = (Bitmask*) calloc(1, sizeof(Bitmask));

    // String handling incoming
    // These delimit the data currently being processed
    char *start_position, *end_position;

    // First find out the width of the map
    start_position = strstr(text, "{WALKMASK}") + strlen("{WALLMASK} ");
    end_position = strstr(start_position, "\n");

    char tmp[end_position - start_position + 1];
    memcpy(tmp, start_position, end_position - start_position);
    tmp[end_position - start_position] = '\0';
    map->width = atoi(tmp);

    // Then the height
    start_position = end_position + 1;
    end_position = strstr(start_position, "\n");

    char tmp2[end_position - start_position + 1];
    memcpy(tmp2, start_position, end_position - start_position);
    tmp2[end_position - start_position] = '\0';
    map->height = atoi(tmp2);

    // Then the actual bit data
    start_position = end_position + 1;
    end_position = strstr(start_position, "\n{END WALKMASK}");

    // First uncompress the data

    // Make the actual 2d array
    map->mask = (bool**) calloc(map->width, sizeof(bool*));
    for (int i=0; i<map->width; i++)
    {
        map->mask[i] = (bool*) calloc(map->height, sizeof(bool));
    }

    // Data is stored in the first 6 bits of every byte, and apparently bottom-right to top-left
    int bitmask = 0x1;
    char* index = end_position - 1;
    int value = ((int) index[0]) - 32;

    // Since it's unlikely that the size of the map is mod 6 == 0, first take out the starting bits from the first byte
    for (int i=0; i<(end_position-start_position)*6 - (map->width*map->height); i++)
    {
        bitmask *= 2;
    }

    for (int j=map->height-1; j>=0; j--)
    {
        for (int i=map->width-1; i>=0; i--)
        {
            if (bitmask == 64)
            {
                // Next/Previous character
                index--;
                value = ((int) index[0]) - 32;
                bitmask = 0x1;
            }
            if (value & bitmask)
            {
                // This part is solid
                map->mask[i][j] = true;
            }
            bitmask *= 2;
        }
    }

    // Deallocate some stuff
    free(text);

    return map;
}
