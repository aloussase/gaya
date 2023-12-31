if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

" Constants
syntax keyword gayaConstant true false
highlight link gayaConstant Constant

" Keywords
syntax keyword gayaKeyword let in do unit cases end given otherwise while perform and or not include when for type of with struct enum upto
highlight link gayaKeyword Keyword

" Literals
syntax region gayaString start=+"+ skip=+\\\\\|\\"+ end=+"+
highlight link gayaString String

" Types
" syntax match gayaType /\v<[A-Z][a-z]*>/ contained containedin=gayaIdentifier
" highlight link gayaType Type

" Identifiers
syntax match gayaIdentifier /\v[A-Za-z0-9_.]+/
highlight link gayaIdentifier Identifier

" Operators
syntax match gayaOperator /::/
syntax match gayaOperator /=>/
syntax match gayaOperator /\v\s+\<-\s+/
syntax match gayaOperator /\s\+\(==\|\/=\|>=\|<=\)\s\+/
syntax match gayaOperator /\v\s+[+-=/*><]\s+/
syntax match gayaOperator /|>/
syntax match gayaOperator /->/
highlight link gayaOperator Operator

" Functions
syntax region gayaFunction start=/\v[a-zA-Z0-9_.]+\(/ end=/)/ contains=ALL
highlight link gayaFunction Function

syntax match gayaNumber /\v[0-9]+(\.[0-9])*/
highlight link gayaNumber Number

" Array
syntax region gayaArray start=/(/ end=/)/ contains=ALL
highlight link gayaArray Structure

" Blocks
syntax region gayaFunctionBody start=/{/ end=/}/ transparent fold

" Comments
syntax keyword gayaTodo TODO contained
syntax region gayaComment start=/\v.*\zs\(\*\ze/ end=/\*)/ contains=gayaTodo
highlight link gayaComment Comment

let b:current_syntax = "gaya"

let &cpo = s:cpo_save
unlet! s:cpo_save
