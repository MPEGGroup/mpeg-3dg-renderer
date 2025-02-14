/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2017, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PROGRAM_OPTIONS_LITE__
#define __PROGRAM_OPTIONS_LITE__

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <utility>
#include <utility>

namespace df
{
  namespace program_options_lite
  {
    struct Options;

    struct ParseFailure : public std::exception
    {
      ParseFailure(std::string arg0, std::string val0) throw()
      : arg(std::move(std::move(arg0))), val(std::move(std::move(val0)))
      {
        printf("ParseFailure: argument: \"%s\" and value: \"%s\" are not correct \n", arg.c_str(), val.c_str() ); fflush(stdout);   
      }

      ~ParseFailure() throw() override = default;;

      std::string arg;
      std::string val;

      const char* what() const throw() override { return "Option Parse Failure"; }
    };

    struct ErrorReporter
    {
      ErrorReporter()  = default;
      virtual ~ErrorReporter() = default;
      virtual std::ostream& error(const std::string& where = "");
      virtual std::ostream& warn(const std::string& where = "");
      bool is_errored = false;
    };

    extern ErrorReporter default_error_reporter;

    void doHelp(std::ostream& out, Options& opts, unsigned columns = 80);
    std::list<const char*> scanArgv(Options& opts, unsigned argc, const char* argv[], ErrorReporter& error_reporter = default_error_reporter);
    void setDefaults(Options& opts);
    void parseConfigFile(Options& opts, const std::string& filename, ErrorReporter& error_reporter = default_error_reporter);

    /* Generic parsing */
    template<typename T>
    inline void
    parse_into(T& dest, const std::string& src)
    {
      std::istringstream src_ss (src, std::istringstream::in);
      src_ss.exceptions(std::ios::failbit);
      src_ss >> dest;
    }

    /** OptionBase: Virtual base class for storing information relating to a
     * specific option This base class describes common elements.  Type specific
     * information should be stored in a derived class. */
    struct OptionBase
    {
      OptionBase(std::string  name, std::string  desc)
      : opt_string(std::move(name)), opt_desc(std::move(desc))
      {};

      virtual ~OptionBase() = default;

      /* parse argument arg, to obtain a value for the option */
      virtual void parse(const std::string& arg, ErrorReporter&) = 0;

      /* set the argument to the default value */
      virtual void setDefault() = 0;

      /* write the default value to out */
      virtual void writeDefault(std::ostream& out) = 0;

      std::string opt_string;
      std::string opt_desc;
    };

    /** Type specific option storage */
    template<typename T>
    struct Option : public OptionBase
    {
      Option(const std::string& name, T& storage, T default_val, const std::string& desc)
      : OptionBase(name, desc), opt_storage(storage), opt_default_val(default_val)
      {}

      void parse(const std::string& arg, ErrorReporter&) override {
        try {
          parse_into(opt_storage, arg);
        }
        catch (...) {
          
          printf("check error\n" ); fflush(stdout);   
          throw ParseFailure(opt_string, arg);
        }
      }

      void setDefault() override
      {
        opt_storage = opt_default_val;
      }

      void writeDefault(std::ostream& out) override
      {
        out << opt_default_val;
      }

      T& opt_storage;
      T opt_default_val;
    };

    /**
     * Container type specific option storage.
     *
     * The option's argument is split by ',' and whitespace.  Runs of
     * whitespace are ignored. Compare:
     *  "a, b,c,,e" = {T1(a), T1(b), T1(c), T1(), T1(e)}, vs.
     *  "a  b c  e" = {T1(a), T1(b), T1(c), T1(e)}.
     *
     * NB: each application of this option overwrites the previous instance,
     *     in exactly the same way that normal (non-container) options to.
     */
    template<template <class, class...> class TT, typename T1, typename... Ts>
    struct Option<TT<T1,Ts...>> : public OptionBase
    {
      typedef TT<T1,Ts...> T;

      Option(const std::string& name, T& storage, T default_val, const std::string& desc)
      : OptionBase(name, desc), opt_storage(storage), opt_default_val(std::move(default_val))
      {}

      void parse(const std::string& arg, ErrorReporter&) override {
        /* ensure that parsing overwrites any previous value */
        opt_storage.clear();

        /* effectively map parse . split m/, /, @arg */
        std::string::size_type pos = 0;
        do {
          /* skip over preceeding spaces */
          pos = arg.find_first_not_of(" \t", pos);
          auto end = arg.find_first_of(", \t", pos);
          std::string sub_arg(arg, pos, end - pos);

          try {
            T1 value;
            parse_into(value, sub_arg);
            opt_storage.push_back(value);
          }
          catch (...) {
            throw ParseFailure(opt_string, sub_arg);
          }

          pos = end + 1;
        } while (pos != std::string::npos + 1);
      }

      void setDefault() override
      {
        opt_storage = opt_default_val;
      }

      void writeDefault(std::ostream& out) override
      {
        out << '"';
        bool first = true;
        for (const auto val : opt_default_val) {
          if (!first) {
            out << ',';
}
          out << val;
          first = false;
        }
        out << '"';
      }

      T& opt_storage;
      T opt_default_val;
    };

    /* string parsing is specialized -- copy the whole string, not just the
     * first word */
    template<>
    inline void
    Option<std::string>::parse(const std::string& arg, ErrorReporter&)
    {
      opt_storage = arg;
    }

    /* strings are pecialized -- output whole string rather than treating as
     * a sequence of characters */
    template<>
    inline void
    Option<std::string>::writeDefault(std::ostream& out)
    {
      out << '"' << opt_default_val << '"';
    }

    /** Option class for argument handling using a user provided function */
    struct OptionFunc : public OptionBase
    {
      using Func = void (Options &, const std::string &, ErrorReporter &);

      OptionFunc(const std::string& name, Options& parent_, std::function<Func> func_, const std::string& desc)
      : OptionBase(name, desc), parent(parent_), func(std::move(std::move(func_)))
      {}

      void parse(const std::string& arg, ErrorReporter& error_reporter) override
      {
        func(parent, arg, error_reporter);
      }

      void setDefault() override
      {
             }

      void writeDefault(std::ostream& out) override
      {
        /* there is no default */
        out << "...";
      }

    private:
      Options& parent;
      std::function<Func> func;
    };

    class OptionSpecific;
    struct Options
    {
      ~Options();

      OptionSpecific addOptions();

      struct Names
      {
        Names()  = default;;
        ~Names()
        {
          
          
            delete opt;
          
        }
        std::list<std::string> opt_long;
        std::list<std::string> opt_short;
        OptionBase* opt = nullptr;
      };

      void addOption(OptionBase *opt);

      using NamesPtrList = std::list<Names *>;
      NamesPtrList opt_list;

      typedef std::map<std::string, NamesPtrList> NamesMap;
      NamesMap opt_long_map;
      NamesMap opt_short_map;
    };

    /* Class with templated overloaded operator(), for use by Options::addOptions() */
    class OptionSpecific
    {
    public:
      OptionSpecific(Options& parent_) : parent(parent_) {}

      /**
       * Add option described by name to the parent Options list,
       *   with storage for the option's value
       *   with default_val as the default value
       *   with desc as an optional help description
       */
      template<typename T>
      OptionSpecific&
      operator()(const std::string& name, T& storage, T default_val, const std::string& desc = "")
      {
        parent.addOption(new Option<T>(name, storage, default_val, desc));
        return *this;
      }

      /**
       * Add option described by name to the parent Options list,
       *   with desc as an optional help description
       * instead of storing the value somewhere, a function of type
       * OptionFunc::Func is called.  It is upto this function to correctly
       * handle evaluating the option's value.
       */
      OptionSpecific&
      operator()(const std::string& name, std::function<OptionFunc::Func> func, const std::string& desc = "")
      {
        parent.addOption(new OptionFunc(name, parent, std::move(func), desc));
        return *this;
      }
    private:
      Options& parent;
    };

  } /* namespace: program_options_lite */
} /* namespace: df */

#endif
