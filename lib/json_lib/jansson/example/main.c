#include <stdio.h>
#include "ARMCM3.h"                     // Device header
#include "jansson.h"                    // Keil::Data Exchange:JSON:Jansson

void add_2array_to_json( json_t* obj, const char* name, const int*
marr, size_t dim1, size_t dim2 )
{
    size_t i, j;
    json_t* jarr1 = json_array();

    for( i=0; i<dim1; ++i ) {
        json_t* jarr2 = json_array();

        for( j=0; j<dim2; ++j ) {
            int val = marr[ i*dim2 + j ];
            json_t* jval = json_integer( val );
            json_array_append_new( jarr2, jval );
        }
        json_array_append_new( jarr1, jarr2 );
    }
    json_object_set_new( obj, name, jarr1 );
    return;
}

int main()
{
    json_t* jdata;
    char* s;
    int arr1[2][3] = { {1,2,3}, {4,5,6} };
    int arr2[4][4] = { {1,2,3,4}, {5,6,7,8}, {9,10,11,12}, {13,14,15,16} };

    jdata = json_object();
    add_2array_to_json( jdata, "arr1", &arr1[0][0], 2, 3 );
    add_2array_to_json( jdata, "arr2", &arr2[0][0], 4, 4 );

    s = json_dumps( jdata, 0 );
    puts( s );
    free( s );
    json_decref( jdata );

    while(1);
}
