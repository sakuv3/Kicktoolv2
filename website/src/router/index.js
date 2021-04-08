import Vue from "vue";
import VueRouter from "vue-router";
import Session from "../components/Session.vue";
import History from "../components/History.vue";
import Prison from "../components/Prison.vue";

Vue.use(VueRouter);

const routes =
    [
        {
            path: '/',
            name: 'root',
            redirect: '/session',
            component: Session,
        },
        {
            path: '/session',
            name: 'Session',
            component: Session,
        },
        {
            path: '/history',
            name: 'History',
            component: History
        },
        {
            path: '/prison',
            name: 'Prison',
            component: Prison
        },
    ]

const router = new VueRouter({
    mode: "history",
    base: process.env.BASE_URL,
    routes
});

export default router;