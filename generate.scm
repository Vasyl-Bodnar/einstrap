(use-modules (scheme base))
(use-modules (rnrs bytevectors))

(define (map-name op val)
  (case op
    ((push) (ash val 3))
    ((pop) (logior 1 (ash val 3)))
    ((add) (logior 2 (ash val 3)))
    ((rep) (logior 3 (ash val 3)))
    ((ext) (logior 4 (ash val 3)))
    ((cop) (logior 5 (ash val 3)))
    ((load) (logior 6 (ash val 3)))
    ((store) (logior 7 (ash val 3)))
    (else (display "Not an op"))))

(define prog
  `((push ,(map-name 'push 3))
    (rep 5)
    (push ,(map-name 'store 3))
    (rep 5)))

(define out (u8-list->bytevector (map (lambda (x) (map-name (car x) (cadr x))) prog)))

(call-with-output-file "out.byt"
  (lambda (port)
    (write-bytevector out port)))

(display out)
