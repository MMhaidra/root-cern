//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_META_PARSER_H
#define CLING_META_PARSER_H

#include "MetaLexer.h" // for cling::Token

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {
  class StringRef;
  namespace sys {
    class Path;
  }
}

namespace cling {
  class MetaLexer;
  class MetaSema;

  // Command syntax: MetaCommand := <CommandSymbol><Command>
  //                 CommandSymbol := '.' | '//.'
  //                 Command := LCommand | XCommand | qCommand | UCommand
  //                            ICommand | RawInputCommand | PrintASTCommand
  //                            DynamicExtensionsCommand | HelpCommand |
  //                            FileExCommand | FilesCommand | ClassCommand
  //                 LCommand := 'L' FilePath
  //                 qCommand := 'q'
  //                 XCommand := 'x' FilePath[ArgList] | 'X' FilePath[ArgList]
  //                 UCommand := 'U'
  //                 ICommand := 'I' [FilePath]
  //                 RawInputCommand := 'rawInput' [Constant]
  //                 PrintASTCommand := 'printAST' [Constant]
  //                 DynamicExtensionsCommand := 'dynamicExtensions' [Constant]
  //                 DynamicExtensionsCommand := 'help'
  //                 DynamicExtensionsCommand := 'fileEx'
  //                 DynamicExtensionsCommand := 'files'
  //                 ClassCommand := 'class' AnyString | 'Class'
  //                 FilePath := AnyString
  //                 ArgList := (ExtraArgList) ' ' [ArgList]
  //                 ExtraArgList := AnyString [, ExtraArgList]
  //                 AnyString := *^(' ' | '\t')
  //                 Constant := 0|1
  //                 Ident := a-zA-Z{a-zA-Z0-9}
  class MetaParser {
  private:
    llvm::OwningPtr<MetaLexer> m_Lexer;
    llvm::OwningPtr<MetaSema> m_Actions;
    llvm::SmallVector<Token, 2> m_TokenCache;
  private:
    inline const Token& getCurTok() { return lookAhead(0); }
    void consumeToken();
    void consumeAnyStringToken(tok::TokenKind stopAt = tok::space);
    const Token& lookAhead(unsigned Num);
    void SkipWhitespace();

    bool isCommandSymbol();
    bool isCommand();
    bool isLCommand();
    bool isExtraArgList();
    bool isXCommand();
    bool isqCommand();
    bool isUCommand();
    bool isICommand();
    bool israwInputCommand();
    bool isprintASTCommand();
    bool isdynamicExtensionsCommand();
    bool ishelpCommand();
    bool isfileExCommand();
    bool isfilesCommand();    
    bool isClassCommand();    
  public:
    MetaParser(MetaSema* Actions);
    void enterNewInputLine(llvm::StringRef Line);
    bool isMetaCommand();
  };
} // end namespace cling

#endif // CLING_META_PARSER_H