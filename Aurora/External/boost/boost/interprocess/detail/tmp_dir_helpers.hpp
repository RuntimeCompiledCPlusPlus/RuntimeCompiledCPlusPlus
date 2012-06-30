//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2007-2011. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_TMP_DIR_HELPERS_HPP
#define BOOST_INTERPROCESS_DETAIL_TMP_DIR_HELPERS_HPP

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/interprocess/errors.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <string>

#if defined(BOOST_INTERPROCESS_WINDOWS)
   //#define BOOST_INTERPROCESS_HAS_WINDOWS_KERNEL_BOOTTIME
   //#define BOOST_INTERPROCESS_HAS_KERNEL_BOOTTIME
   //#include <boost/interprocess/detail/win32_api.hpp>
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
   //#include <sys/sysctl.h>
   //#if defined(CTL_KERN) && defined (KERN_BOOTTIME)
      //#define BOOST_INTERPROCESS_HAS_BSD_KERNEL_BOOTTIME
      //#define BOOST_INTERPROCESS_HAS_KERNEL_BOOTTIME
   //#endif
#endif

namespace boost {
namespace interprocess {
namespace ipcdetail {

#if defined (BOOST_INTERPROCESS_HAS_WINDOWS_KERNEL_BOOTTIME)
inline void get_bootstamp(std::string &s, bool add = false)
{
   std::string bootstamp;
   winapi::get_last_bootup_time(bootstamp);
   if(add){
      s += bootstamp;
   }
   else{
      s.swap(bootstamp);
   }
}
#elif defined(BOOST_INTERPROCESS_HAS_BSD_KERNEL_BOOTTIME)
inline void get_bootstamp(std::string &s, bool add = false)
{
   // FreeBSD specific: sysctl "kern.boottime"
   int request[2] = { CTL_KERN, KERN_BOOTTIME };
   struct ::timeval result;
   std::size_t result_len = sizeof result;

   if (::sysctl (request, 2, &result, &result_len, NULL, 0) < 0)
      return;

   char bootstamp_str[256];

   const char Characters [] =
      { '0', '1', '2', '3', '4', '5', '6', '7'
      , '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

   std::size_t char_counter = 0;
   //32 bit values to allow 32 and 64 bit process IPC
   boost::uint32_t fields[2] = { boost::uint32_t(result.tv_sec), boost::uint32_t(result.tv_usec) };
   for(std::size_t field = 0; field != 2; ++field){
      for(std::size_t i = 0; i != sizeof(fields[0]); ++i){
         const char *ptr = (const char *)&fields[field];
         bootstamp_str[char_counter++] = Characters[(ptr[i]&0xF0)>>4];
         bootstamp_str[char_counter++] = Characters[(ptr[i]&0x0F)];
      }
   }
   bootstamp_str[char_counter] = 0;
   if(add){
      s += bootstamp_str;
   }
   else{
      s = bootstamp_str;
   }
}
#endif

inline void get_tmp_base_dir(std::string &tmp_name)
{
   #if defined (BOOST_INTERPROCESS_WINDOWS)
   winapi::get_shared_documents_folder(tmp_name);
   if(tmp_name.empty() || !winapi::is_directory(tmp_name.c_str())){
      tmp_name = get_temporary_path();
   }
   #else
   tmp_name = get_temporary_path();
   #endif
   if(tmp_name.empty()){
      error_info err = system_error_code();
      throw interprocess_exception(err);
   }
   //Remove final null.
   tmp_name += "/boost_interprocess";
}

inline void tmp_folder(std::string &tmp_name)
{
   get_tmp_base_dir(tmp_name);
   #ifdef BOOST_INTERPROCESS_HAS_KERNEL_BOOTTIME
   tmp_name += "/";
   get_bootstamp(tmp_name, true);
   #endif
}

inline void tmp_filename(const char *filename, std::string &tmp_name)
{
   tmp_folder(tmp_name);
   tmp_name += "/";
   tmp_name += filename;
}

inline void create_tmp_and_clean_old(std::string &tmp_name)
{
   //First get the temp directory
   std::string root_tmp_name;
   get_tmp_base_dir(root_tmp_name);

   //If fails, check that it's because already exists
   if(!create_directory(root_tmp_name.c_str())){
      error_info info(system_error_code());
      if(info.get_error_code() != already_exists_error){
         throw interprocess_exception(info);
      }
   }

   #ifdef BOOST_INTERPROCESS_HAS_KERNEL_BOOTTIME
   tmp_folder(tmp_name);

   //If fails, check that it's because already exists
   if(!create_directory(tmp_name.c_str())){
      error_info info(system_error_code());
      if(info.get_error_code() != already_exists_error){
         throw interprocess_exception(info);
      }
   }
   //Now erase all old directories created in the previous boot sessions
   std::string subdir = tmp_name;
   subdir.erase(0, root_tmp_name.size()+1);
   delete_subdirectories(root_tmp_name, subdir.c_str());
   #else
   tmp_name = root_tmp_name;
   #endif
}

inline void create_tmp_and_clean_old_and_get_filename(const char *filename, std::string &tmp_name)
{
   create_tmp_and_clean_old(tmp_name);
   tmp_name += "/";
   tmp_name += filename;
}

inline void add_leading_slash(const char *name, std::string &new_name)
{
   if(name[0] != '/'){
      new_name = '/';
   }
   new_name += name;
}

}  //namespace boost{
}  //namespace interprocess {
}  //namespace ipcdetail {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //ifndef BOOST_INTERPROCESS_DETAIL_TMP_DIR_HELPERS_HPP
