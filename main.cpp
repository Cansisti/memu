#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <string>
#include <iomanip>

#include "marie.h"

#define mem_size (4096*2)

using namespace std;

int main( int argc, char ** argv ) {

	if( argc < 2 ) {
		cout << "no file" << endl;
		return -1;
    }

    bool verbose = false, memdump = false, dissasm = false;
    uint16_t md_b, md_e;
    for( int i = 2; i<argc; i++ ) {
		if( string( argv[i] ) == "-v" ) verbose = true;
		if( string( argv[i] ) == "-d" ) dissasm = true;
        if( string( argv[i] ).find( "-m" ) == 0 ) {
			string x = string( argv[i] );
			x.erase( 0, 2 );
			size_t p = x.find( ":" );
			if( p == string::npos ) {
				cout << "specify memory dump range: -mB:E (in hex)" << endl;
				return -1;
			}
			md_b = stoul( x.substr( 0, p ), nullptr, 16 );
			md_e = stoul( x.substr( p+1, x.size()-p-1 ), nullptr, 16 );
			if( md_b > md_e ) {
				cout << "memory dump cannot begin further than it ends" << endl;
				return -1;
			}
			memdump = true;
        }
    }

    ifstream in( argv[1] );
    if( !in.is_open() ) return -2;

    uint16_t * mem = (uint16_t*) malloc( mem_size );

    memset( mem, 0x0, mem_size );
    in.read( (char*) mem, mem_size );

    if( in.bad() ) {
		cout << "memory too small!" << endl;
		return -2;
    }

    bool brk = false;

	int16_t ac = 0;
	int16_t ir;

    for( uint16_t pc = 0; pc < mem_size; ) {
		ir = (mem[pc] & 0x0fff);
		if( verbose ) cout << (mem[pc] & 0xf000) / 0x0fff << (dissasm ? " " : "\n");
        switch( mem[pc] & 0xf000 ) {
        	case add: {
        		if( dissasm ) cout << "add " << ir << endl;
				ac += mem[ ir ];
				break;
        	}
			case subt: {
				if( dissasm ) cout << "subt " << ir << endl;
				ac -= mem[ ir ];
				break;
			}
			case addl: {
				if( dissasm ) cout << "addl " << ir << endl;
				ac -= mem[ mem[ ir ] ];
				break;
			}
			case clear_: {
				if( dissasm ) cout << "clear" << endl;
				ac = 0;
				break;
			}
			case load: {
				if( dissasm ) cout << "load " << ir << endl;
				ac = mem[ ir ];
				break;
			}
			case store: {
				if( dissasm ) cout << "store " << ir << endl;
				mem[ ir ] = ac;
				break;
			}
			case input: {
				if( dissasm ) cout << "input" << endl;
				cout << "<< ";
				cin >> hex >> ac;
				break;
			}
			case output: {
				if( dissasm ) cout << "output" << endl;
				cout << ">> ";
				cout << hex << ac << endl;
				break;
			}
			case jump: {
				if( dissasm ) cout << "jump " << ir << endl;
				pc = ir;
				break;
			}
			case skipcond: {
				if( dissasm ) cout << "skipcond " << ir << endl;
				switch( ir ) {
					case 0x0000: if( ac < 0 ) pc++; break;
					case 0x0400: if( ac == 0 ) pc++; break;
					case 0x0800: if( ac > 0 ) pc++; break;
				}
				break;
			}
			case jns: {
				if( dissasm ) cout << "jns " << ir << endl;
				mem[ ir ] = pc;
				pc = ir + 1;
				break;
			}
			case jumpl: {
				if( dissasm ) cout << "jumpl " << ir << endl;
				pc = mem[ ir ];
				break;
			}
			case halt: {
				if( dissasm ) cout << "halt" << endl;
				brk = true;
				break;
			}
        }
        //if( (verbose || dissasm) && !( (mem[pc] & 0xf000) == input ) || ( (mem[pc] & 0xf000) == output ) ) cout << endl;
		pc++;
        if( brk ) break;
    }

    if( memdump ) {
		cout << endl << "[Memory dump]" << endl;
		cout << setfill( '0' );
		for( int16_t m = md_b; m<md_e+1; m++ ) {
			if( (m-md_b) % 8 == 0 ) cout << "[" << setw( 3 ) << m << "]: ";
			cout << setw( 4 ) << mem[m] << " ";
			if( (m-md_b) % 8 == 7 ) cout << endl;
		}
    }

    return 0;
}
