/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if !defined(SYMTAB_SWK_H_)
#define SYMTAB_SWK_H_

#if defined(cap_stackwalker_use_symtab)

#include "stackwalk/h/framestepper.h"
#include "symtabAPI/h/Symtab.h"
#include "dynutil/h/dyntypes.h"
#include "stackwalk/h/procstate.h"
#include "symtabAPI/h/AddrLookup.h"
#include "symtabAPI/h/Function.h"
#include <string>

using namespace Dyninst::SymtabAPI;

namespace Dyninst {
namespace Stackwalker {

class swkProcessReader : public ProcessReader {
 private:
   ProcessState *procstate;
 public:
   swkProcessReader(ProcessState *pstate, const std::string& executable);
   virtual bool start();
   virtual bool ReadMem(Address inTraced, void *inSelf, unsigned amount);
   virtual bool GetReg(Dyninst::MachRegister, Dyninst::MachRegisterVal&) { return false; }
   virtual bool done();
   virtual ~swkProcessReader();
};

class SymtabLibState : public LibraryState {
 private:
   bool needs_update;
   AddressLookup *lookup;
   swkProcessReader procreader;
   bool updateLibs();
   bool updateLibsArch();
   bool refresh();
   std::vector<std::pair<LibAddrPair, unsigned> > arch_libs;
#if defined(os_linux)
 private:
   void *vsyscall_mem;
   Symtab *vsyscall_symtab;
   LibAddrPair vsyscall_libaddr;
   enum {vsys_unset, vsys_set, vsys_none, vsys_error} vsyscall_page_set;
 public:
   Symtab *getVsyscallSymtab();
   bool getVsyscallLibAddr(LibAddrPair &vsys);
#endif

#if defined(os_aix) || defined(os_linux)
 public:
   bool getLibc(LibAddrPair &addr_pair);
   bool getLibpthread(LibAddrPair &addr_pair);
#endif

 public:
   SymtabLibState(ProcessState *parent, std::string executable = "");
   virtual bool getLibraryAtAddr(Address addr, LibAddrPair &olib);
   virtual bool getLibraries(std::vector<LibAddrPair> &olibs);
   virtual void notifyOfUpdate();
   virtual Address getLibTrapAddress();
   virtual ~SymtabLibState();
   LibAddrPair getAOut();
};

class SymtabWrapper {
 private:
   dyn_hash_map<std::string, Symtab *> map;
   static SymtabWrapper *wrapper;
 protected:
   SymtabWrapper();
 public:
   static Symtab *getSymtab(std::string filename);
   static void notifyOfSymtab(Symtab *symtab, std::string name);
   ~SymtabWrapper();
};

class DyninstInstrStepperImpl : public FrameStepper {
 private:
   static std::map<Symtab *, bool> isRewritten;
   FrameStepper *parent;
  
 public:
   DyninstInstrStepperImpl(Walker *w, FrameStepper *p);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   gcframe_ret_t getCallerFrameArch(const Frame &in, Frame &out, Address base, Address lib_base, 
				    unsigned size, unsigned stack_height);
   virtual unsigned getPriority() const;
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~DyninstInstrStepperImpl();
};

}
}

#endif
#endif