- Sync with commit [5b6fc5b7](https://gitlab.com/rpncalculators/c43/-/commits/master?ref_type=HEADS)
- Fixed bug related to [this one](https://gitlab.com/rpncalculators/c43/-/commit/adf655d182aba034f77323954b0b046b9297b17a) which would cause the app to freeze during a very intense calculation.
You can test that the app behaves as expected by running this complex calculation

```
2 ENTER
325
y^x
1
-
f-shift CAT
FACTORS
```

**NOTE:** You should be able to interrupt the calculation by pressing `EXIT`
