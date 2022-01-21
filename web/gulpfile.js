'use strict';


var fs = require('fs');
var gulp = require('gulp');
var debug = require('gulp-debug');
var nunjucksRender = require('gulp-nunjucks-render');
var del = require('del');
var sourcemaps = require('gulp-sourcemaps');
var terser = require('gulp-terser');
var htmlmin = require('gulp-htmlmin');
var concat = require('gulp-concat');
var gulpIf = require("gulp-if");
var csso = require('gulp-csso');

const isDev = !!(!process.env.NODE_ENV || process.env.NODE_ENV === "dev");
console.warn("Current mode is " + (isDev ? "development" : "production"));

const DIST_DIR = "../dist";

var paths = {
    css: [
        "css/bootstrap.css",
        "css/style.css"
    ],
    js: [
        "js/bootstrap.js",
        "js/main.js"
    ],
    html: "*.+(html|nunjucks)"
};

gulp.task('cleanDist', function () {
    return del(DIST_DIR+'/**/*',{force:true});
});

gulp.task('nunjucks', function() {
    return gulp.src(paths.html)
        .pipe(nunjucksRender({
            path: ['templates']
        }))
        .pipe(gulpIf(isDev, debug({title: 'nunjucks:'})))
        .pipe(htmlmin({collapseWhitespace: true, removeComments: true}))
        .pipe(gulpIf(isDev, debug({title: 'htmlmin:'})))
        .pipe(gulp.dest(DIST_DIR))
});

gulp.task('minifycss', function () {
    return gulp.src(paths.css)
        .pipe(gulpIf(isDev, sourcemaps.init()))
        .pipe(csso({ restructure: true, comments: false }))
        .pipe(gulpIf(isDev, debug({title: 'css_csso:'})))
        .pipe(concat('style.css'))
        .pipe(gulpIf(isDev, sourcemaps.write()))
        .pipe(gulpIf(isDev, debug({title: 'css_min:'})))
        .pipe(gulp.dest(DIST_DIR));
});

gulp.task('uglifyjs', function () {
    return gulp.src(paths.js)
        .pipe(gulpIf(isDev, sourcemaps.init()))
        .pipe(terser({ output: {comments: false} }))
        .pipe(gulpIf(isDev, debug({title: 'js_min:'})))
        .pipe(concat('script.js'))
        .pipe(gulpIf(isDev, sourcemaps.write()))
        .pipe(gulpIf(isDev, debug({title: 'js_concat:'})))
        .pipe(gulp.dest(DIST_DIR));
});

gulp.task('createHPP', function (cb) {
    const hpp_file = '../static.hpp';

    fs.writeFileSync(hpp_file, "#pragma once\r\n", { encoding: "utf8", flag: 'w' });
    console.log("Created HPP file");

    const css_string = fs.readFileSync(DIST_DIR+'/style.css', {encoding:'utf8', flag:'r'});
    fs.writeFileSync(hpp_file, `#define CSS_FILE_STRING "${JSON.stringify(css_string).slice(1, -1)}"\r\n`, { encoding: "utf8", flag: "a+" });
    console.log("Written CSS code");

    const js_string  = fs.readFileSync(DIST_DIR+'/script.js', {encoding:'utf8', flag:'r'});
    fs.writeFileSync(hpp_file, `#define JS_FILE_STRING "${JSON.stringify(js_string).slice(1, -1)}"\r\n`, { encoding: "utf8", flag: "a+" });
    console.log("Written JS code");

    return cb();
});

gulp.task("default", gulp.series("cleanDist", gulp.parallel("nunjucks", "minifycss", "uglifyjs")));