if exists("b:did_ftplugin")
  finish
endif

let b:did_ftplugin = 1

let s:cpo_orig = &cpo
set cpo&vim

setlocal expandtab
setlocal tabstop=8
setlocal softtabstop=2
setlocal shiftwidth=2

if has('comments')
    setlocal comments=:(*
    setlocal commentstring=(*\ %s\ *)
endif

let &cpo = s:cpo_orig
unlet s:cpo_orig
