// @(#)root/treeplayer:$Id$
// Author: Axel Naumann, 2011-09-21

/*************************************************************************
 * Copyright (C) 1995-2013, Rene Brun and Fons Rademakers and al.        *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TTreeReader.h"

#include "TChain.h"
#include "TDirectory.h"
#include "TTreeReaderValue.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// TTreeReader                                                                //
//                                                                            //
// Connects TTreeReaderValue to a TTree.                                   //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

ClassImp(TTreeReader)

//______________________________________________________________________________
TTreeReader::TTreeReader(TTree* tree):
   fTree(tree),
   fDirectory(0),
   fEntryStatus(kEntryNotLoaded),
   fDirector(0)
{
   // Access data from tree.
   Initialize();
}

//______________________________________________________________________________
TTreeReader::TTreeReader(const char* keyname, TDirectory* dir /*= NULL*/):
   fTree(0),
   fDirectory(dir),
   fEntryStatus(kEntryNotLoaded),
   fDirector(0)
{
   // Access data from the tree called keyname in the directory (e.g. TFile)
   // dir, or the current directory if dir is NULL. If keyname cannot be
   // found, or if it is not a TTree, IsZombie() will return true.
   if (!fDirectory) fDirectory = gDirectory;
   fDirectory->GetObject(keyname, fTree);
   Initialize();
}

//______________________________________________________________________________
TTreeReader::~TTreeReader()
{
   // Tell all value readers that the tree reader does not exist anymore.
   for (std::deque<ROOT::TTreeReaderValueBase*>::const_iterator
           i = fValues.begin(), e = fValues.end(); i != e; ++i) {
      (*i)->MarkTreeReaderUnavailable();
   }
   delete fDirector;
   fProxies.SetOwner();
}

//______________________________________________________________________________
void TTreeReader::Initialize()
{
   // Initialization of the director.
   if (!fTree) {
      MakeZombie();
      fEntryStatus = kEntryNoTree;
   } else {
      fDirector = new ROOT::TBranchProxyDirector(fTree, -1);
   }
}

//______________________________________________________________________________
Long64_t TTreeReader::GetCurrentEntry() const {
   //Returns the index of the current entry being read

   if (!fDirector) return 0;
   Long64_t currentTreeEntry = fDirector->GetReadEntry();
   if (fTree->IsA() == TChain::Class() && currentTreeEntry >= 0) {
      return ((TChain*)fTree)->GetChainEntryNumber(currentTreeEntry);
   }
   return currentTreeEntry;
}

//______________________________________________________________________________
TTreeReader::EEntryStatus TTreeReader::SetEntryBase(Long64_t entry, Bool_t local)
{
   // Load an entry into the tree, return the status of the read.
   // For chains, entry is the global (i.e. not tree-local) entry number.

   if (!fTree) {
      fEntryStatus = kEntryNoTree;
      return fEntryStatus;
   }

   TTree* prevTree = fDirector->GetTree();

   int loadResult;
   if (!local){
      Int_t treeNumInChain = fTree->GetTreeNumber();

      loadResult = fTree->LoadTree(entry);

      if (loadResult == -2) {
         fEntryStatus = kEntryNotFound;
         return fEntryStatus;
      }

      Int_t currentTreeNumInChain = fTree->GetTreeNumber();
      if (treeNumInChain != currentTreeNumInChain) {
            fDirector->SetTree(fTree->GetTree());
      }
   }
   else {
      loadResult = entry;
   }
   if (!prevTree || fDirector->GetReadEntry() == -1) {
      // Tell readers we now have a tree
      for (std::deque<ROOT::TTreeReaderValueBase*>::const_iterator
              i = fValues.begin(); i != fValues.end(); ++i) { // Iterator end changes when parameterized arrays are read
         (*i)->CreateProxy();

         if (!(*i)->GetProxy()){
            fEntryStatus = kEntryDictionaryError;
            return fEntryStatus;
         }
      }
   }
   fDirector->SetReadEntry(loadResult);
   fEntryStatus = kEntryValid;
   return fEntryStatus;
}

//______________________________________________________________________________
void TTreeReader::SetTree(TTree* tree)
{
   // Set (or update) the which tree to reader from. tree can be
   // a TTree or a TChain.
   fTree = tree;
   if (fTree) {
      ResetBit(kZombie);
      if (fTree->InheritsFrom(TChain::Class())) {
         SetBit(kBitIsChain);
      }
   }

   if (!fDirector) {
      Initialize();
   }
   else {
      fDirector->SetTree(fTree);
      fDirector->SetReadEntry(-1);
   }
}

//______________________________________________________________________________
void TTreeReader::RegisterValueReader(ROOT::TTreeReaderValueBase* reader)
{
   // Add a value reader for this tree.
   fValues.push_back(reader);
}

//______________________________________________________________________________
void TTreeReader::DeregisterValueReader(ROOT::TTreeReaderValueBase* reader)
{
   // Remove a value reader for this tree.
   std::deque<ROOT::TTreeReaderValueBase*>::iterator iReader
      = std::find(fValues.begin(), fValues.end(), reader);
   if (iReader == fValues.end()) {
      Error("DeregisterValueReader", "Cannot find reader of type %s for branch %s", reader->GetDerivedTypeName(), reader->fBranchName.Data());
      return;
   }
   fValues.erase(iReader);
}
