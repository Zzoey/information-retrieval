# Description

This project introduces a concurrent application for information retrieval, using the [Standard Boolean Model](https://en.wikipedia.org/wiki/Standard_Boolean_model).
More precisely, this implementation offers the possibility of parallel query processing, over the [Cranfield Collection](http://ir.dcs.gla.ac.uk/resources/test_collections/cran/) of text documents, using **Atomic Memory Transactions** implemented in **C++**.

#### Standard Boolean Model

Based on Boolean logic and classical set theory, the Boolean Model corresponds documents and queries to set of terms. As a result, retrieval is based on whether documents contain query terms or not.

For example, given a set of documents *Doc<sub>i</sub>* and a query *Q*:

* *Doc<sub>1</sub>* -> {*word<sub>1</sub>, word<sub>2</sub>, word<sub>3</sub>*}
* *Doc<sub>2</sub>* -> {*word<sub>2</sub>, word<sub>3</sub>*}
* *Doc<sub>3</sub>* -> {*word<sub>3</sub>*}
* *Q* -> {*word<sub>1</sub>, word<sub>2</sub>, word<sub>3</sub>*}

The Boolean model would evaluate the documents as follows:

* *Doc<sub>1</sub>* -> score = 3 (contains 3 terms)
* *Doc<sub>2</sub>* -> score = 2 (contains 2 terms)
* *Doc<sub>3</sub>* -> score = 1 (contains 3 terms)

#### Cranfield Collection

The test collection of Cranfield includes 1400 abstracts of aeronautical journal articles, a set of 225 queries, and exhaustive relevance evaluations of all (query, document) pairs. 

---

# Pre-Processing

Initially, the Cranfield collection was stored in two files:

* *cran.all.1400*, which contains 1400 abstracts of aeronautical journal articles
* *cran.qry*, which contains 225 relevant queries

In order to facilitate parallel processing, documents and queries are splitted to 1400 text files for the documents and 225 for the queries.

Apart from splitting, the [SnowballAnalyzer](https://lucene.apache.org/core/4_0_0/analyzers-common/org/apache/lucene/analysis/snowball/SnowballAnalyzer.html) and [StopAnalyzer](https://lucene.apache.org/core/4_0_0/analyzers-common/org/apache/lucene/analysis/core/StopAnalyzer.html) classes of [Apache Lucene](https://lucene.apache.org/) are used for stemming and stop-words removal.
