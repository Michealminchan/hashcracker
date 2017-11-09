# hashcracker

Much like John the Ripper, without all the fancy shit. Just reads from a wordlist and cracks hashes.

## Build

Simply:

``` sh
make
```

Then run:

```
$ ./hashcrack 
Creating 8 threads
hashchcker

Usage:   hashchcker [options]

Options: --wordlist, -w [filename]: The wordlist to check against
         --format,   -f [hash_fun]: The hash function (md5|sha|sha1|sha256|sha512)
         --target,   -t [hash]:     The hash to check
         --newlines, -n:            Whether to hash with newlines
         --amount,   -a:            Amount of hashing
         --help,     -h:            Prints this message
```

Example usage:

```
./hashcrack -w english.txt -f sha256 -t 9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08
```

The same when searching for a string with newlines at the end:

```
./hashcrack -w english.txt -f sha256 -t f2ca1bb6c7e907d06dafe4687e579fce76b37e4e93b7605022da52e6ccc26fd2 -n
```

The above commands should output something like the following:

```
Creating 4 threads
Reading /home/conmarap/Desktop/Labs/lab-crypto2/tools/SecLists/Passwords/english.txt and producing sha256 with newlines
The result is (with a newline): test
Found it after 301649 tries and 1.953693 seconds!
```

Then there's the amount of nesting. In other words, a hash within a hash. In the example below we have the word 'zeta' hashed twice.

```
./hashcrack --wordlist ../SecLists/Passwords/english.txt --format sha512 --target 9bfdb2e82597de7542396cc07cad208053e2d769f31024d8e8c552a58e59e908b4e4392a9436d459005cbbb46547c341d6b07aa31ebcecc2955fe5db19fbfe09 -a 2
```

This should return the following:

```
Creating 8 threads
Reading ../SecLists/Passwords/english.txt and producing sha512
Hashing 2 times.
The result is (without a newline): zeta
Found it after 346561 tries and 9.854559 seconds!
```

## Wodlists

Either build one with [John The Ripper](https://github.com/magnumripper/JohnTheRipper):

```
john --wordlist=wordlist.txt --rules --stdout > mutated.txt
```

Or get em from [here](https://github.com/danielmiessler/SecLists/tree/master/Passwords).

## FAQ

### But, why?

Because I am...

![awesome](https://media.giphy.com/media/T2MuGuH3u1eeI/giphy.gif)


