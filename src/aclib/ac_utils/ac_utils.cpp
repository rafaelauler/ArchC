/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_utils.cpp
 * @author    Sandro Rigo
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     ArchC support functions
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "ac_utils.H"

#ifdef USE_GDB
#include "ac_gdb.H"
// extern AC_GDB *gdbstub;
#endif /* USE_GDB */

#ifdef AC_VERIFY

//Declaring co-verification message queue
// key_t key;
// int msqid;


#endif

// int ac_stop_flag = 0;
// int ac_exit_status = 0;

//Declaring trace variables
ofstream trace_file;
bool ac_do_trace;

//Name of the file containing the application to be loaded.
//char *appfilename;
std::map<std::string, std::ofstream*> ac_cache_traces;

//Read model options before application
void ac_init_opts( int ac, char* av[]){

    extern const char *project_name;
    extern const char *project_file;
    extern const char *archc_version;
    extern const char *archc_options;

    if(ac > 1){

        if (!strncmp( av[1], "--help", 6)) {
            cerr << "This is a " << project_name << " architecture simulator generated by ArchC.\n";
            cerr << "For more information visit http://www.archc.org\n\n";
            cerr << "Usage: " << av[0] << " [options]\n\n";
            cerr << "Options:\n";
            cerr << "  --help                  Display this help message\n";
            cerr << "  --version               Display ArchC version and options used when built\n";
            cerr << "  --load=<prog_path>      Load target application\n";
            cerr << "  -- <prog_path>          Load target application\n";
            cerr << "  --trace-cache=<cache>,<file> Trace cache access\n";
#ifdef USE_GDB
            //      cerr << "  --gdb[=<port>]          Enable GDB support\n";
#endif /* USE_GDB */
            exit(1);
        }

        if (!strncmp( av[1], "--version", 9)) {
            cout << project_name << " simulator generated by ArchC " << archc_version
                << " from " << project_file << " with options (" << archc_options << ")" << endl;
            exit(0);
        }
    }
}

char* appfilename = NULL;
//Initialize arguments for application
args_t ac_init_args( int ac, char* av[]){

    int size;
    char *appname=0;
    int ac_argc;
    char** ac_argv;

    ac_argc = ac-1;   //Skiping program's name
    ac_argv = av;

    // Check if "help" or "version" was called
    ac_init_opts(ac, av);

    while(ac > 1){
        size = strlen(av[1]);

        //Checking if an application needs to be loaded
        if( (size>11) && (!strncmp( av[1], "--load_obj=", 11))){   // Loading hex file in the ArchC Format
            appname = (char*)malloc(size - 10);
            strcpy(appname, av[1]+11);
            ac_argv[1] = appname;
            appfilename = (char*) malloc(sizeof(char)*(size - 6));
            strcpy(appfilename, appname);
            //ac_load_obj( appname );
        }
        else if( (size>7) && (!strncmp( av[1], "--load=", 7))){  // Loading a binary app with former way: --load=<bin>
            appname = (char*)malloc(size - 6);
            strcpy(appname, av[1]+7);
            ac_argv[1] = appname;
            appfilename = (char*) malloc(sizeof(char)*(size - 6));
            strcpy(appfilename, appname);

        } else if ( (size==2) && (!strncmp( av[1], "--", 2))) { // Loading a binary with new way: -- <bin> 
            ac_argc--;
            ac--;

            // Remove -- from ac_argv
            for (int i = 1; i < ac; i++) {
                ac_argv[i] = ac_argv[i + 1];
            }

            size = strlen(av[1]);
            appname = (char *)malloc(size + 1);
            strcpy(appname, av[1]);
            // ac_argv[1] = appname;
            appfilename = (char *)malloc(sizeof(char) * (size + 1));
            strcpy(appfilename, appname);

            av++;
        }
#ifdef USE_GDB
        //     if( (size>=5) && (!strncmp( av[1], "--gdb", 5))){ //Enable GDB support
        //       int port = 0;
        //       if ( size > 6 )
        //         {
        //           port = atoi( av[1] + 6 );
        //           if ( ( port > 1024 ) && gdbstub )
        //             gdbstub->set_port( port );
        //         }
        //       if ( gdbstub )
        //         gdbstub->enable();
        //     }
#endif /* USE_GDB */

        else if ( (size>14) && (!strncmp(av[1], "--trace-cache=", 14)) ) {
            char *comma = strchr(av[1], ',');
            if (comma == NULL) {
                std::cerr << "Error: invalid argument syntax.\n";
                exit(EXIT_FAILURE);
            }
            std::string cache_name(av[1]+14, comma);
            std::string file_name(comma+1, av[1]+size);
            ac_cache_traces[cache_name] = new std::ofstream(file_name.c_str());
            if (!ac_cache_traces[cache_name]) {
                std::cerr << "Error opening file: " << file_name << "\n";
                exit(EXIT_FAILURE);
            }
            // Remove this parameter from the list and reset the loop
            for (int i = 1; i <= ac; i++) {
                av[i] = av[i+1];
            }

            ac_argc--;
            ac--;
            continue;
        }

        ac --;
        av ++;
    }

    if (!appname) {
        AC_ERROR("No application provided.");
        AC_ERROR("Use --load=<prog_path> or -- <prog_path> to load a target application.");
        AC_ERROR("Try --help for more options.");
        exit(1);
    }
    ac_argv++;

    args_t args;
    args.size = ac_argc;
    args.app_args = ac_argv;
    args.app_filename = appfilename;
    return args;
}

unsigned int convert_endian(unsigned int size, unsigned int num, bool match_endian)
{
  unsigned char *in = (unsigned char*) &num;
  unsigned int out = 0;

  if (! match_endian) {
    for(; size>0; size--) {
      out <<= 8;
      out |= in[0];
      in++;
    }
  }
  else {
    out = num;
  }

  return out;
}
